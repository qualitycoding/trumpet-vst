// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "PluginProcessor.h"
#include "TrumpetView.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

/// Editor: trumpet view on top, controls below (D-013, D-016). 60 Hz timer copies the UI snapshot into the view.
class TrumpetAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit TrumpetAudioProcessorEditor(TrumpetAudioProcessor&);
    ~TrumpetAudioProcessorEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    /// Pulls the current UI state into the view immediately (also called by the timer). Used by tests.
    void refreshFromProcessor();
    TrumpetView& view() { return view_; }

private:
    void timerCallback() override { refreshFromProcessor(); }
    TrumpetAudioProcessor& processor_;
    TrumpetView view_;
    std::array<juce::Slider, 9> knobs_;
    std::array<juce::Label, 9> knobLabels_;
    juce::ToggleButton alternates_{"Alternate fingerings"}, fixedValves_{"Fixed valves"};
    juce::Label buildLabel_;
    bool unsupported_ = false;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 9> knobAttachments_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> alternatesAttachment_, fixedAttachment_;
};
