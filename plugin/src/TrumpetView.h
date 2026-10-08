// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/Layout.h"
#include <juce_gui_basics/juce_gui_basics.h>

/// Programmatic drawing of the trumpet with valves, slide triggers, harmonic ladder and caption (D-013).
class TrumpetView : public juce::Component {
public:
    TrumpetView();
    /// Stores the state and repaints only when it changed.
    void setState(const tpt::UiState& s);
    const tpt::UiState& state() const { return state_; }
    void paint(juce::Graphics& g) override;

private:
    tpt::UiState state_;
};
