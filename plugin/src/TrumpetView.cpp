// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-014.
#include "TrumpetView.h"

TrumpetView::TrumpetView() = default;
void TrumpetView::setState(const tpt::UiState& s) { state_ = s; }
void TrumpetView::paint(juce::Graphics& g) { g.fillAll(juce::Colours::black); }
