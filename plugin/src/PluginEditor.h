// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "PluginProcessor.h"
#include "TrumpetView.h"
#include <juce_audio_processors/juce_audio_processors.h>

/// Editor: trumpet view on top, controls below (D-013, D-016). 60 Hz timer copies the UI snapshot into the view.
class TrumpetAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit TrumpetAudioProcessorEditor(TrumpetAudioProcessor&);
    ~TrumpetAudioProcessorEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
    /// Pulls the current UI state into the view immediately (also called by the timer). Used by tests.
    void refreshFromProcessor();
    TrumpetView& view() { return view_; }

private:
    void timerCallback() override { refreshFromProcessor(); }
    TrumpetAudioProcessor& processor_;
    TrumpetView view_;
};
