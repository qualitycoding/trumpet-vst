// SPDX-License-Identifier: Apache-2.0
#include "PluginEditor.h"

#ifndef TPT_GIT_SHA
#define TPT_GIT_SHA "unknown"
#endif

namespace {
struct KnobSpec { const char* id; const char* label; };
constexpr KnobSpec kKnobs[9] = {{tptparams::kBrightness, "Brightness"},   {tptparams::kLipStiffness, "Lip stiffness"},
                                {tptparams::kBreathNoise, "Breath noise"}, {tptparams::kVibratoRate, "Vibrato rate"},
                                {tptparams::kVibratoDepth, "Vibrato depth"}, {tptparams::kTuning, "Tuning A4"},
                                {tptparams::kIntonation, "Intonation"},    {tptparams::kGain, "Gain (dB)"},
                                {tptparams::kOverblow, "Overblow"}};
constexpr int kViewHeight = 400;
} // namespace

TrumpetAudioProcessorEditor::TrumpetAudioProcessorEditor(TrumpetAudioProcessor& p) : AudioProcessorEditor(p), processor_(p) {
    addAndMakeVisible(view_);
    for (std::size_t i = 0; i < 9; ++i) {
        auto& k = knobs_[i];
        k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        k.setNumDecimalPlacesToDisplay(2);
        addAndMakeVisible(k);
        knobAttachments_[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters(), kKnobs[i].id, k);
        knobLabels_[i].setText(kKnobs[i].label, juce::dontSendNotification);
        knobLabels_[i].setJustificationType(juce::Justification::centred);
        knobLabels_[i].setColour(juce::Label::textColourId, juce::Colour(0xFFE6E8EB));
        addAndMakeVisible(knobLabels_[i]);
    }
    addAndMakeVisible(alternates_);
    addAndMakeVisible(fixedValves_);
    alternatesAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters(), tptparams::kAlternates, alternates_);
    fixedAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters(), tptparams::kFixedValves, fixedValves_);
    buildLabel_.setText(juce::String("Build ") + TPT_GIT_SHA, juce::dontSendNotification);
    buildLabel_.setJustificationType(juce::Justification::centredRight);
    buildLabel_.setColour(juce::Label::textColourId, juce::Colour(0xFF9AA0A6));
    buildLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(buildLabel_);
    setSize(960, 600);
    refreshFromProcessor();
    startTimerHz(60);
}

TrumpetAudioProcessorEditor::~TrumpetAudioProcessorEditor() { stopTimer(); }

void TrumpetAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    view_.setBounds(area.removeFromTop(kViewHeight));
    auto controls = area.reduced(10, 6);
    auto footer = controls.removeFromBottom(18);
    buildLabel_.setBounds(footer.removeFromRight(260));
    auto toggles = controls.removeFromRight(170);
    alternates_.setBounds(toggles.removeFromTop(toggles.getHeight() / 2).reduced(4));
    fixedValves_.setBounds(toggles.reduced(4));
    const int w = controls.getWidth() / 9;
    for (std::size_t i = 0; i < 9; ++i) {
        auto col = controls.removeFromLeft(w);
        knobLabels_[i].setBounds(col.removeFromTop(18));
        knobs_[i].setBounds(col);
    }
}

void TrumpetAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xFF23272E)); }

void TrumpetAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
    if (!unsupported_) return;
    auto r = view_.getBounds();
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillRect(r);
    g.setColour(juce::Colour(0xFFFFB000));
    g.setFont(juce::Font(juce::FontOptions(28.0f)));
    g.drawText("unsupported sample rate", r, juce::Justification::centred, false);
}

void TrumpetAudioProcessorEditor::refreshFromProcessor() {
    view_.setState(processor_.currentUiState());
    const bool unsupported = processor_.unsupportedRate();
    if (unsupported != unsupported_) {
        unsupported_ = unsupported;
        repaint();
    }
}
