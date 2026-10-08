// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/// Parameter IDs and ranges (D-016). Version hint 1 for all parameters.
namespace tptparams {
inline constexpr const char* kBrightness = "brightness";
inline constexpr const char* kLipStiffness = "lipStiffness";
inline constexpr const char* kBreathNoise = "breathNoise";
inline constexpr const char* kVibratoRate = "vibratoRate";
inline constexpr const char* kVibratoDepth = "vibratoDepth";
inline constexpr const char* kTuning = "tuning";
inline constexpr const char* kIntonation = "intonation";
inline constexpr const char* kGain = "gain";
inline constexpr const char* kOverblow = "overblow";
inline constexpr const char* kAlternates = "alternates";
inline constexpr const char* kFixedValves = "fixedValves";

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout() {
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&](const char* id, const char* name, float lo, float hi, float def) {
        l.add(std::make_unique<AudioParameterFloat>(ParameterID{id, 1}, name, NormalisableRange<float>(lo, hi), def));
    };
    f(kBrightness, "Brightness", 0.0f, 1.0f, 0.5f);
    f(kLipStiffness, "Lip stiffness", 0.0f, 1.0f, 0.5f);
    f(kBreathNoise, "Breath noise", 0.0f, 1.0f, 0.1f);
    f(kVibratoRate, "Vibrato rate", 3.0f, 8.0f, 5.5f);
    f(kVibratoDepth, "Vibrato depth", 0.0f, 1.0f, 0.0f);
    f(kTuning, "Tuning A4", 415.0f, 466.0f, 440.0f);
    f(kIntonation, "Intonation realism", 0.0f, 1.0f, 0.0f);
    f(kGain, "Output gain", -24.0f, 12.0f, 0.0f);
    f(kOverblow, "Overblow", -1.0f, 1.0f, 0.0f);
    l.add(std::make_unique<AudioParameterBool>(ParameterID{kAlternates, 1}, "Alternate fingerings", false));
    l.add(std::make_unique<AudioParameterBool>(ParameterID{kFixedValves, 1}, "Fixed valves", false));
    return l;
}
} // namespace tptparams
