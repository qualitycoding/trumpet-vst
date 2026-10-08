// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "Parameters.h"
#include "tpt/TrumpetVoice.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

/// Mono trumpet instrument (D-010 .. D-016). MIDI: notes 24..31 keyswitches (D-003), CC2/CC11 breath, CC1 and
/// channel pressure vibrato, CC16 Overblow, pitch bend +-2 semitones, CC120/CC123 all notes off; omni.
class TrumpetAudioProcessor : public juce::AudioProcessor {
public:
    TrumpetAudioProcessor();
    ~TrumpetAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Trumpet"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& parameters() { return apvts_; }
    /// UI snapshot from the voice (lock-free; D-013). Idle state before prepareToPlay().
    tpt::UiState currentUiState() const;

private:
    juce::AudioProcessorValueTreeState apvts_;
    std::unique_ptr<tpt::TrumpetVoice> voice_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrumpetAudioProcessor)
};
