// SPDX-License-Identifier: Apache-2.0
#include "tpt/Fingering.h"
#include "tpt/Pitch.h"
#include <cmath>

namespace tpt {

const std::vector<Fingering>& fingeringTable() {
    static const std::vector<Fingering> table = {
#include "FingeringTable.inc"
    };
    return table;
}

namespace {
std::optional<Fingering> lookup(int written, bool alternate) {
    for (const auto& f : fingeringTable())
        if (f.written == written && f.alternate == alternate) return f;
    return std::nullopt;
}
constexpr int kLowestConcert = 28, kHighestConcert = 100;
} // namespace

std::optional<Fingering> standardFingering(int written) { return lookup(written, false); }
std::optional<Fingering> alternateFingering(int written) { return lookup(written, true); }

std::optional<ResolvedNote> resolveNote(int concertMidi, bool useAlternates, std::optional<Valves> fixedValves) {
    if (concertMidi < kLowestConcert || concertMidi > kHighestConcert) return std::nullopt;
    ResolvedNote r;
    if (fixedValves) {
        const Valves v = *fixedValves;
        const double target = concertMidi + kTransposition;
        int bestN = 1;
        double bestD = 1e9, bestNominal = 0.0;
        for (int n = 1; n <= kMaxPartial; ++n) {
            const double nominal = 48.0 + 12.0 * std::log2(double(n)) - loweringSemitones(v);
            const double d = std::fabs(nominal - target);
            if (d < bestD - 1e-12) { bestD = d; bestN = n; bestNominal = nominal; }
        }
        r.written = static_cast<int>(std::lround(bestNominal));
        r.concert = r.written - kTransposition;
        r.fingering = Fingering{r.written, v, bestN, Trigger::None, false};
        r.extended = r.written > kHighestNormalWritten;
        r.fixedValves = true;
        return r;
    }
    const int written = concertMidi + kTransposition;
    if (written < kLowestWritten || written > kHighestWritten) return std::nullopt;
    auto f = useAlternates ? alternateFingering(written) : std::nullopt;
    if (!f) f = standardFingering(written);
    if (!f) return std::nullopt;
    r.concert = concertMidi;
    r.written = written;
    r.fingering = *f;
    r.extended = written > kHighestNormalWritten;
    return r;
}

} // namespace tpt
