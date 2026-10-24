// SPDX-License-Identifier: Apache-2.0
#include "tpt/Valves.h"
#include <array>
#include <stdexcept>

namespace tpt {

int loweringSemitones(Valves v) {
    if (v > 7) throw std::out_of_range("loweringSemitones: mask > 7");
    return ((v & kValve1) ? 2 : 0) + ((v & kValve2) ? 1 : 0) + ((v & kValve3) ? 3 : 0);
}

std::string valvesName(Valves v) {
    if (v > 7) throw std::out_of_range("valvesName: mask > 7");
    if (v == 0) return "0";
    std::string s;
    if (v & kValve1) s += '1';
    if (v & kValve2) s += '2';
    if (v & kValve3) s += '3';
    return s;
}

std::optional<Valves> parseValves(std::string_view name) {
    for (Valves v = 0; v < kValveCombinationCount; ++v)
        if (valvesName(v) == name) return v;
    return std::nullopt;
}

std::optional<Valves> keyswitchValves(int midiNote) {
    if (midiNote < kKeyswitchLow || midiNote > kKeyswitchHigh) return std::nullopt;
    static const std::array<Valves, 8> order = {0, kValve2, kValve1, kValve1 | kValve2, kValve2 | kValve3,
                                                kValve1 | kValve3, kValve1 | kValve2 | kValve3, kValve3};
    return order[static_cast<size_t>(midiNote - kKeyswitchLow)];
}

} // namespace tpt
