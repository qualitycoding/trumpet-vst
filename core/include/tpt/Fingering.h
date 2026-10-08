// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/Valves.h"
#include <optional>
#include <vector>

namespace tpt {

/// One fingering of the B-flat trumpet (D-002). `written` is the written MIDI pitch, `partial` the harmonic of the
/// valve combination's natural series (open partial 2 = written C4 = 60).
struct Fingering {
    int written = 0;
    Valves valves = 0;
    int partial = 0;
    Trigger trigger = Trigger::None;
    bool alternate = false;
    bool operator==(const Fingering&) const = default;
};

inline constexpr int kLowestWritten = 54;        // F#3
inline constexpr int kHighestNormalWritten = 84; // C6 (A-002 normal range)
inline constexpr int kHighestWritten = 89;       // F6 (extended range)
inline constexpr int kMaxPartial = 13;           // highest partial addressable by fixed valves / overblow

/// Every fingering, standard and alternate, in the order of tests/fixtures/trumpet_fingerings.txt. The table is
/// compiled in (not read from the fixture at run time).
const std::vector<Fingering>& fingeringTable();

/// Standard fingering for a written pitch in [kLowestWritten, kHighestWritten], else std::nullopt.
std::optional<Fingering> standardFingering(int written);

/// Alternate fingering for a written pitch, if the table has one, else std::nullopt.
std::optional<Fingering> alternateFingering(int written);

/// Result of mapping an incoming concert-pitch MIDI note to what the virtual trumpeter plays (D-002, D-003).
struct ResolvedNote {
    int concert = 0;          ///< concert MIDI note that will sound nominally (fixed valves: the partial's nominal note)
    int written = 0;          ///< concert + kTransposition
    Fingering fingering;      ///< the fingering used
    bool extended = false;    ///< written > kHighestNormalWritten
    bool fixedValves = false; ///< resolved in Fixed-valves mode
    bool operator==(const ResolvedNote&) const = default;
};

/// Normal mode (fixedValves = nullopt): written = concert + 2 must lie in [kLowestWritten, kHighestWritten], else
/// nullopt; uses the alternate fingering when useAlternates is true and one exists, else the standard one.
/// Fixed-valves mode: the fingering uses the held valves and the partial n in [1, kMaxPartial] whose nominal
/// written pitch 48 + 12 log2(n) - loweringSemitones(valves) is nearest to the requested written pitch (ties -> lower
/// n); written/concert are that nominal pitch rounded to the nearest integer; trigger None; alternate false.
/// Requested concert notes outside [28, 100] -> nullopt in both modes.
std::optional<ResolvedNote> resolveNote(int concertMidi, bool useAlternates, std::optional<Valves> fixedValves);

} // namespace tpt
