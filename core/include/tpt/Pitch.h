// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>

namespace tpt {

/// B-flat trumpet: written pitch = concert pitch + 2 semitones (A-003).
inline constexpr int kTransposition = 2;

/// 12-TET frequency in Hz: a4Hz * 2^((midi - 69) / 12). Any integer midi (no range check).
double equalTemperedHz(int midi, double a4Hz = 440.0);

/// 1200 * log2(f / ref). Requires f > 0 and ref > 0, otherwise throws std::invalid_argument.
double centsBetween(double f, double ref);

/// Note name with sharps and octave number, MIDI 60 = "C4": e.g. 54 -> "F#3", 70 -> "A#4". midi in [0, 127],
/// otherwise throws std::out_of_range.
std::string noteName(int midi);

} // namespace tpt
