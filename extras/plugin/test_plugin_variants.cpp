// SPDX-License-Identifier: Apache-2.0
// Non-frozen plugin tests (S-013): held valves in the state (R-014), non-NUL-terminated state blobs (R-016), unsupported
// host rates (R-022), CC11 and transport start (R-033), CC68 legato pedal (R-035).
#include "PluginProcessor.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
constexpr int kBlock = 256;

struct FakeHead : juce::AudioPlayHead {
    bool playing = false;
    juce::Optional<PositionInfo> getPosition() const override {
        PositionInfo info;
        info.setIsPlaying(playing);
        return info;
    }
};

juce::AudioBuffer<float> run(TrumpetAudioProcessor& p, int blocks, juce::MidiBuffer first = {}) {
    juce::AudioBuffer<float> all(2, blocks * kBlock), buf(2, kBlock);
    for (int b = 0; b < blocks; ++b) {
        juce::MidiBuffer m;
        if (b == 0) m = first;
        buf.clear();
        p.processBlock(buf, m);
        for (int c = 0; c < 2; ++c) all.copyFrom(c, b * kBlock, buf, c, 0, kBlock);
    }
    return all;
}

juce::MidiBuffer msg(std::initializer_list<juce::MidiMessage> ms) {
    juce::MidiBuffer b;
    for (const auto& m : ms) b.addEvent(m, 0);
    return b;
}

void prepare(TrumpetAudioProcessor& p, double fs = 48000.0) {
    p.setPlayConfigDetails(0, 2, fs, kBlock);
    p.prepareToPlay(fs, kBlock);
}

float rms(const juce::AudioBuffer<float>& b, int start, int n) { return b.getRMSLevel(0, start, n); }
float db(float x) { return 20.0f * std::log10(std::max(x, 1e-9f)); }

/// Lowest 5 ms RMS (dB) in the first `ms` milliseconds of x.
float minWindowDb(const juce::AudioBuffer<float>& x, int ms) {
    const int w = 240;
    float lo = 1e9f;
    for (int s = 0; s + w <= ms * 48; s += w) lo = std::min(lo, db(rms(x, s, w)));
    return lo;
}
} // namespace

TEST_CASE("variant: held valves are written to the state and restored; an invalid value is ignored", "[variant]") {
    TrumpetAudioProcessor a;
    prepare(a);
    a.parameters().getParameter("fixedValves")->setValueNotifyingHost(1.0f);
    run(a, 2, msg({juce::MidiMessage::noteOn(1, 29, (juce::uint8) 100)}));       // keyswitch: valves 1-3
    CHECK(a.currentUiState().valves == (tpt::kValve1 | tpt::kValve3));
    juce::MemoryBlock mb;
    a.getStateInformation(mb);
    const std::string text(static_cast<const char*>(mb.getData()), mb.getSize());
    CHECK(text.find("\"heldValves\":\"13\"") != std::string::npos);

    TrumpetAudioProcessor b;
    prepare(b);
    b.setStateInformation(mb.getData(), static_cast<int>(mb.getSize()));
    run(b, 2);
    CHECK(b.currentUiState().valves == (tpt::kValve1 | tpt::kValve3));

    std::string bad = text;
    bad.replace(bad.find("\"13\""), 4, "\"9\"");
    TrumpetAudioProcessor c;
    prepare(c);
    c.setStateInformation(bad.data(), static_cast<int>(bad.size()));
    run(c, 2);
    CHECK(c.currentUiState().valves == 0);                                         // invalid heldValves ignored
}

TEST_CASE("variant: a state blob without a terminating NUL is read by its size", "[variant]") {
    TrumpetAudioProcessor a, b;
    a.parameters().getParameter("brightness")->setValueNotifyingHost(0.9f);
    juce::MemoryBlock mb;
    a.getStateInformation(mb);
    std::vector<char> exact(static_cast<const char*>(mb.getData()), static_cast<const char*>(mb.getData()) + mb.getSize());
    REQUIRE(!exact.empty());
    CHECK(exact.back() == '}');                                                    // no NUL
    b.setStateInformation(exact.data(), static_cast<int>(exact.size()));
    CHECK(std::fabs(dynamic_cast<juce::AudioParameterFloat*>(b.parameters().getParameter("brightness"))->get() - 0.9f) < 1e-4f);
    b.setStateInformation(exact.data(), 0);                                        // zero size: ignored
    b.setStateInformation(nullptr, 10);
}

TEST_CASE("variant: an unsupported host rate gives silence and is reported", "[variant]") {
    TrumpetAudioProcessor p;
    prepare(p, 8000.0);
    CHECK(p.unsupportedRate());
    auto out = run(p, 20, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 100)}));
    CHECK(rms(out, 0, out.getNumSamples()) == 0.0f);
    prepare(p, 48000.0);
    CHECK_FALSE(p.unsupportedRate());
    out = run(p, 60, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 100)}));
    CHECK(rms(out, 0, out.getNumSamples()) > 0.005f);
    prepare(p, 384000.0);
    CHECK(p.unsupportedRate());
}

TEST_CASE("variant: CC11 = 0 is undone by a transport start", "[variant]") {
    TrumpetAudioProcessor p;
    prepare(p);
    FakeHead head;
    p.setPlayHead(&head);
    auto out = run(p, 60, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 100)}));
    const float level = rms(out, out.getNumSamples() - 4800, 4800);
    REQUIRE(level > 0.01f);
    out = run(p, 20, msg({juce::MidiMessage::controllerEvent(1, 11, 0)}));
    CHECK(rms(out, out.getNumSamples() - 2400, 2400) < 0.05f * level);             // faded out
    head.playing = true;                                                           // transport start
    out = run(p, 4);
    CHECK(rms(out, out.getNumSamples() - 256, 256) > 0.5f * level);                // level restored
    p.setPlayHead(nullptr);
}

TEST_CASE("variant: CC68 slurs touching notes, without it they are tongued", "[variant]") {
    auto dip = [](bool pedal) {
        TrumpetAudioProcessor p;
        prepare(p);
        if (pedal) run(p, 1, msg({juce::MidiMessage::controllerEvent(1, 68, 127)}));
        run(p, 160, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 100)}));
        const auto steadyBlock = run(p, 20);
        const float steady = db(rms(steadyBlock, 0, steadyBlock.getNumSamples()));
        // touching notes: note-off and the next note-on at one sample position
        const auto y = run(p, 24, msg({juce::MidiMessage::noteOff(1, 67), juce::MidiMessage::noteOn(1, 72, (juce::uint8) 100)}));
        return minWindowDb(y, 80) - steady;
    };
    CHECK(dip(true) > -20.0f);       // legato: no articulation dip
    CHECK(dip(false) < -8.0f);       // tongued
}

int main(int argc, char* argv[]) {
    juce::ScopedJuceInitialiser_GUI juce;
    return Catch::Session().run(argc, argv);
}
