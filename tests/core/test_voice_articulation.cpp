// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-035 re-articulation (pre-mortem R-029, D-010): a note-on with no other note held is tongued, even while the previous
// note still rings; a same-pitch repeat is tongued. Thresholds from research/spikes/tongue_spike.py (C-101): 25 ms stop
// gives dips of 11.7-20.1 dB and recovery within 3 dB in a median 95 ms (97 % <= 250 ms); after the D-007 40 ms release a
// 100 ms gap dips 16.9-56.4 dB (research/spikes/release_gap_spike.py, C-102: only 87 % reach 20 dB, so 12 dB is required).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>

using namespace tpttest;

namespace {
constexpr double kFs = 48000;
constexpr size_t kWin = 240;   // 5 ms RMS windows

double winDb(const std::vector<float>& x, size_t start) { return tpt::rmsDb(x.data() + start, kWin); }

/// Steady level (median of 5 ms windows over 0.4-0.6 s of the first note).
double steadyDb(const std::vector<float>& x) {
    std::vector<double> v;
    for (size_t k = static_cast<size_t>(0.4 * kFs); k + kWin <= static_cast<size_t>(0.6 * kFs); k += kWin) v.push_back(winDb(x, k));
    std::nth_element(v.begin(), v.begin() + static_cast<long>(v.size() / 2), v.end());
    return v[v.size() / 2];
}
} // namespace

TEST_CASE("T-035 same-pitch repeats and detached notes are re-tongued (normal range, mf)", "[T-035][integration]") {
    int notes = 0, recovered = 0;
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
        const int c = f.written - 2;
        // (a) repeat with zero gap: note-off and note-on of the same pitch at the same instant
        {
            auto v = makeVoice(kFs);
            v->noteOn(c, kVelMF);
            auto x = render(*v, kFs, 0.6);
            v->noteOff(c);
            v->noteOn(c, kVelMF);
            const auto y = render(*v, kFs, 0.6);
            const double steady = steadyDb(x);
            double minDb = 1e9;
            for (size_t k = 0; k + kWin <= static_cast<size_t>(0.06 * kFs); k += kWin) minDb = std::min(minDb, winDb(y, k));
            INFO("repeat, written " << f.written << " steady " << steady << " min " << minDb);
            CHECK(minDb <= steady - 8.0);
            // recovery: from 300 ms on, every 5 ms window is within 3 dB of steady
            bool ok = true;
            for (size_t k = static_cast<size_t>(0.3 * kFs); k + kWin <= y.size(); k += kWin) ok = ok && winDb(y, k) >= steady - 3.0;
            ++notes; recovered += ok;
            CHECK(nominalPartialOf(f0Of(tail(y, kFs, 0.2), kFs), f.valves) == f.partial);
        }
        // (b) detached: 100 ms gap, then the same note again
        {
            auto v = makeVoice(kFs);
            v->noteOn(c, kVelMF);
            auto x = render(*v, kFs, 0.6);
            v->noteOff(c);
            const auto gap = render(*v, kFs, 0.1);
            const double steady = steadyDb(x);
            double minDb = 1e9;
            for (size_t k = 0; k + kWin <= gap.size(); k += kWin) minDb = std::min(minDb, winDb(gap, k));
            INFO("gap, written " << f.written);
            CHECK(minDb <= steady - 12.0);
            v->noteOn(c, kVelMF);
            const auto y = render(*v, kFs, 0.6);
            CHECK(nominalPartialOf(f0Of(tail(y, kFs, 0.2), kFs), f.valves) == f.partial);
        }
    }
    CHECK(recovered >= static_cast<int>(std::ceil(0.9 * notes)));
}

TEST_CASE("T-035 a note-on while another note is held stays legato", "[T-035][integration]") {
    auto v = makeVoice(kFs);
    v->noteOn(58, kVelMF);                // written C4, open p2
    auto x = render(*v, kFs, 0.6);
    v->noteOn(65, kVelMF);                // held 58 -> legato lip slur to written G4
    const auto y = render(*v, kFs, 0.4);
    const double steady = steadyDb(x);
    for (size_t k = 0; k + kWin <= y.size(); k += kWin) {
        INFO("window " << k);
        CHECK(winDb(y, k) > steady - 20.0);
    }
}
