// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-013. Renders silence and does not create the voice (the embedded table is empty until
// S-006), so that the stub plugin loads and passes pluginval.
#include "PluginProcessor.h"
#include "PluginEditor.h"

TrumpetAudioProcessor::TrumpetAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "TrumpetState", tptparams::createLayout()) {}
TrumpetAudioProcessor::~TrumpetAudioProcessor() = default;

void TrumpetAudioProcessor::prepareToPlay(double, int) {}
void TrumpetAudioProcessor::releaseResources() {}
bool TrumpetAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}
void TrumpetAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
}
juce::AudioProcessorEditor* TrumpetAudioProcessor::createEditor() { return new TrumpetAudioProcessorEditor(*this); }
void TrumpetAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void TrumpetAudioProcessor::setStateInformation(const void*, int) {}
tpt::UiState TrumpetAudioProcessor::currentUiState() const { return {}; }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TrumpetAudioProcessor(); }
