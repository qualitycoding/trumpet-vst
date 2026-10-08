// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-014 (controls, attachments, timer).
#include "PluginEditor.h"

TrumpetAudioProcessorEditor::TrumpetAudioProcessorEditor(TrumpetAudioProcessor& p) : AudioProcessorEditor(p), processor_(p) {
    addAndMakeVisible(view_);
    setSize(960, 600);
}
TrumpetAudioProcessorEditor::~TrumpetAudioProcessorEditor() = default;
void TrumpetAudioProcessorEditor::resized() { view_.setBounds(getLocalBounds().removeFromTop(400)); }
void TrumpetAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colours::darkgrey); }
void TrumpetAudioProcessorEditor::refreshFromProcessor() {}
