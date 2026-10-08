// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace tpt {

/// Valve combination as a bit mask: bit 0 = valve 1, bit 1 = valve 2, bit 2 = valve 3 (D-002). 0 = open.
using Valves = std::uint8_t;
inline constexpr Valves kValve1 = 1, kValve2 = 2, kValve3 = 4;
inline constexpr int kValveCombinationCount = 8;

/// Slide trigger in use for a fingering: none, first-valve slide, third-valve slide (D-002, D-013).
enum class Trigger : std::uint8_t { None = 0, First = 1, Third = 3 };

/// Nominal lowering in equal-tempered semitones: valve 2 = 1, valve 1 = 2, valve 3 = 3, summed for combinations.
/// Mask > 7 throws std::out_of_range.
int loweringSemitones(Valves v);

/// Canonical name: "0" (open) or the pressed valves in ascending order, e.g. "1", "13", "123". Mask > 7 throws
/// std::out_of_range.
std::string valvesName(Valves v);

/// Inverse of valvesName (accepts exactly the 8 canonical names); anything else -> std::nullopt.
std::optional<Valves> parseValves(std::string_view name);

/// Keyswitch notes for Fixed-valves mode (D-003): MIDI 24..31 select, in this order, "0", "2", "1", "12", "23",
/// "13", "123", "3". Returns std::nullopt for any other note.
std::optional<Valves> keyswitchValves(int midiNote);

/// Lowest and highest keyswitch note (inclusive).
inline constexpr int kKeyswitchLow = 24, kKeyswitchHigh = 31;

} // namespace tpt
