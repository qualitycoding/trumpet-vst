// SPDX-License-Identifier: Apache-2.0
#include "tpt/Pitch.h"
#include <cmath>
#include <stdexcept>

namespace tpt {

double equalTemperedHz(int midi, double a4Hz) { return a4Hz * std::pow(2.0, (midi - 69) / 12.0); }

double centsBetween(double f, double ref) {
    if (!(f > 0.0) || !(ref > 0.0)) throw std::invalid_argument("centsBetween: frequencies must be positive");
    return 1200.0 * std::log2(f / ref);
}

std::string noteName(int midi) {
    if (midi < 0 || midi > 127) throw std::out_of_range("noteName: midi out of [0, 127]");
    static const char* const names[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return std::string(names[midi % 12]) + std::to_string(midi / 12 - 1);
}

} // namespace tpt
