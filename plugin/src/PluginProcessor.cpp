// SPDX-License-Identifier: Apache-2.0
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "tpt/State.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>

namespace {
constexpr double kMinRate = 22050.0, kMaxRate = 192000.0;
constexpr int kMaxStateBytes = 64 * 1024;

std::shared_ptr<const tpt::ResonatorTable> embeddedTable() {
    return std::make_shared<const tpt::ResonatorTable>(tpt::ResonatorTable::fromJson(tpt::embeddedResonatorJson()));
}
} // namespace

TrumpetAudioProcessor::TrumpetAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "TrumpetState", tptparams::createLayout()),
      voice_(std::make_unique<tpt::TrumpetVoice>(embeddedTable())) {
    const char* ids[11] = {tptparams::kBrightness, tptparams::kLipStiffness, tptparams::kBreathNoise, tptparams::kVibratoRate,
                           tptparams::kVibratoDepth, tptparams::kTuning,      tptparams::kIntonation, tptparams::kGain,
                           tptparams::kOverblow,     tptparams::kAlternates,  tptparams::kFixedValves};
    for (int i = 0; i < 11; ++i) raw_[i] = apvts_.getRawParameterValue(ids[i]);
}
TrumpetAudioProcessor::~TrumpetAudioProcessor() = default;

tpt::VoiceParameters TrumpetAudioProcessor::readParameters() const {
    tpt::VoiceParameters p;
    p.brightness = raw_[0]->load();
    p.lipStiffness = raw_[1]->load();
    p.breathNoise = raw_[2]->load();
    p.vibratoRateHz = raw_[3]->load();
    p.vibratoDepth = raw_[4]->load();
    p.tuningA4Hz = raw_[5]->load();
    p.intonationRealism = raw_[6]->load();
    p.outputGainDb = raw_[7]->load();
    p.overblow = raw_[8]->load();
    p.useAlternates = raw_[9]->load() > 0.5f;
    p.fixedValves = raw_[10]->load() > 0.5f;
    return p;
}

void TrumpetAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    sampleRate_ = sampleRate;
    prepared_ = false;
    unsupported_ = !(sampleRate >= kMinRate && sampleRate <= kMaxRate);
    if (unsupported_) {
        setLatencySamples(0);
        return;
    }
    maxChunk_ = std::min(8192, std::max(samplesPerBlock, 512));
    voice_->prepare(sampleRate, maxChunk_);
    setLatencySamples(voice_->latencySamples());
    expressionGain_ = expressionTarget_ = 1.0f;
    expressionCoeff_ = static_cast<float>(1.0 - std::exp(-1.0 / (0.010 * sampleRate)));
    wasPlaying_ = false;
    prepared_ = true;
}

void TrumpetAudioProcessor::releaseResources() {}

bool TrumpetAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void TrumpetAudioProcessor::handleMidi(const juce::MidiMessage& m) {
    if (m.isNoteOff()) {                       // includes note-on with velocity 0
        voice_->noteOff(m.getNoteNumber());
    } else if (m.isNoteOn()) {
        const int note = m.getNoteNumber();
        if (!voice_->keyswitch(note)) voice_->noteOn(note, m.getFloatVelocity());
    } else if (m.isAllNotesOff() || m.isAllSoundOff()) {                 // CC123, CC120
        voice_->allNotesOff();
    } else if (m.isPitchWheel()) {
        voice_->setPitchBend(2.0f * static_cast<float>(m.getPitchWheelValue() - 8192) / 8192.0f);
    } else if (m.isChannelPressure()) {
        pressure_ = static_cast<float>(m.getChannelPressureValue()) / 127.0f;
        voice_->setVibratoControl(std::max(modWheel_, pressure_));
    } else if (m.isController()) {
        const float v = static_cast<float>(m.getControllerValue());
        switch (m.getControllerNumber()) {
        case 1: modWheel_ = v / 127.0f; voice_->setVibratoControl(std::max(modWheel_, pressure_)); break;
        case 2: voice_->setBreath(v / 127.0f); break;
        case 11: expressionTarget_ = v / 127.0f; break;
        case 16: voice_->setOverblowControl(std::clamp((v - 64.0f) / 63.0f, -1.0f, 1.0f)); break;
        case 68: voice_->setLegatoPedal(m.getControllerValue() >= 64); break;
        case 121:                                                       // reset controllers
            voice_->setBreath(-1.0f);
            voice_->setPitchBend(0.0f);
            modWheel_ = pressure_ = 0.0f;
            voice_->setVibratoControl(0.0f);
            voice_->releaseOverblowControl();
            expressionTarget_ = 1.0f;
            break;
        default: break;
        }
    }
}

void TrumpetAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (!prepared_ || unsupported_ || buffer.getNumChannels() < 1) {
        buffer.clear();
        return;
    }
    // transport start: CC11 gain back to 1 and no breath controller (D-014, R-030, R-033), before this block's events
    bool playing = false;
    if (auto* head = getPlayHead())
        if (auto pos = head->getPosition()) playing = pos->getIsPlaying();
    if (playing && !wasPlaying_) {
        expressionTarget_ = expressionGain_ = 1.0f;
        voice_->setBreath(-1.0f);
    }
    wasPlaying_ = playing;

    voice_->setParameters(readParameters());

    float* out = buffer.getWritePointer(0);
    int pos = 0;
    auto render = [&](int upTo) {
        while (pos < upTo) {
            const int chunk = std::min(upTo - pos, maxChunk_);
            voice_->process(out + pos, chunk);
            pos += chunk;
        }
    };
    auto it = midi.begin();
    while (it != midi.end()) {
        const int at = std::clamp((*it).samplePosition, 0, n);
        render(at);
        // events at one sample position: note-offs and panics first (touching notes are tongued unless CC68), then the rest
        auto groupEnd = it;
        while (groupEnd != midi.end() && std::clamp((*groupEnd).samplePosition, 0, n) == at) ++groupEnd;
        for (auto e = it; e != groupEnd; ++e) {
            const auto m = (*e).getMessage();
            if (m.isNoteOff() || m.isAllNotesOff() || m.isAllSoundOff()) handleMidi(m);
        }
        for (auto e = it; e != groupEnd; ++e) {
            const auto m = (*e).getMessage();
            if (!(m.isNoteOff() || m.isAllNotesOff() || m.isAllSoundOff())) handleMidi(m);
        }
        it = groupEnd;
    }
    render(n);

    for (int i = 0; i < n; ++i) {
        expressionGain_ += expressionCoeff_ * (expressionTarget_ - expressionGain_);
        out[i] *= expressionGain_;
    }
    for (int c = 1; c < buffer.getNumChannels(); ++c) buffer.copyFrom(c, 0, out, n);
    midi.clear();
}

juce::AudioProcessorEditor* TrumpetAudioProcessor::createEditor() { return new TrumpetAudioProcessorEditor(*this); }

void TrumpetAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto j = nlohmann::json::parse(tpt::serializeState(readParameters()), nullptr, false);
    if (j.is_discarded()) return;
    j["heldValves"] = tpt::valvesName(voice_->heldValves());
    const std::string text = j.dump();
    destData.append(text.data(), text.size());
}

void TrumpetAudioProcessor::applyParametersToState(const tpt::VoiceParameters& p) {
    auto setFloat = [&](const char* id, float v) {
        if (auto* prm = apvts_.getParameter(id)) prm->setValueNotifyingHost(prm->convertTo0to1(v));
    };
    auto setBool = [&](const char* id, bool v) {
        if (auto* prm = apvts_.getParameter(id)) prm->setValueNotifyingHost(v ? 1.0f : 0.0f);
    };
    setFloat(tptparams::kBrightness, p.brightness);
    setFloat(tptparams::kLipStiffness, p.lipStiffness);
    setFloat(tptparams::kBreathNoise, p.breathNoise);
    setFloat(tptparams::kVibratoRate, p.vibratoRateHz);
    setFloat(tptparams::kVibratoDepth, p.vibratoDepth);
    setFloat(tptparams::kTuning, p.tuningA4Hz);
    setFloat(tptparams::kIntonation, p.intonationRealism);
    setFloat(tptparams::kGain, p.outputGainDb);
    setFloat(tptparams::kOverblow, p.overblow);
    setBool(tptparams::kAlternates, p.useAlternates);
    setBool(tptparams::kFixedValves, p.fixedValves);
}

void TrumpetAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // sized view: the host blob is not NUL terminated (D-026 R-016)
    if (data == nullptr || sizeInBytes <= 0 || sizeInBytes > kMaxStateBytes) return;
    const std::string_view text(static_cast<const char*>(data), static_cast<std::size_t>(sizeInBytes));
    const auto p = tpt::deserializeState(text);
    if (!p) return;                                                     // invalid state: parameters unchanged
    applyParametersToState(*p);
    const auto j = nlohmann::json::parse(text.begin(), text.end(), nullptr, false);
    if (j.is_object()) {
        const auto it = j.find("heldValves");
        if (it != j.end() && it->is_string())
            if (const auto v = tpt::parseValves(it->get<std::string>())) voice_->setHeldValves(*v);   // invalid value ignored
    }
}

tpt::UiState TrumpetAudioProcessor::currentUiState() const { return voice_->uiState(); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TrumpetAudioProcessor(); }
