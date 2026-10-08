// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-021 dynamics: louder is louder and brighter (C-074, D-008). T-022 brassiness: ff gains high harmonics (C-074,
// C-085, D-012). Thresholds are below the reference spread (reference: 100 % ordering, H6-10 ff-pp q10 = 13.7 dB).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <numeric>

using namespace tpttest;

TEST_CASE("T-021 / T-022 dynamics and brassiness (normal range)", "[T-021][T-022][integration]") {
    const double fs = 48000;
    int notes = 0, ordered = 0, brassy = 0;
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
        const int concert = f.written - 2;
        const double f0 = tpt::equalTemperedHz(concert);
        double rms[3], cen[3], hi[3];
        int i = 0;
        for (float vel : {kVelPP, kVelMF, kVelFF}) {
            const auto x = tail(renderNote(fs, concert, vel, 1.0), fs, 0.5);
            rms[i] = tpt::rmsDb(x.data(), x.size());
            cen[i] = tpt::spectralCentroidHz(x.data(), x.size(), fs) / f0;
            const auto h = tpt::harmonicLevelsDb(x.data(), x.size(), fs, f0, 10);
            hi[i] = std::accumulate(h.begin() + 5, h.end(), 0.0) / 5.0;   // harmonics 6..10
            ++i;
        }
        INFO("written " << f.written << " rms " << rms[0] << "/" << rms[1] << "/" << rms[2]);
        CHECK(rms[0] < rms[1]);
        CHECK(rms[1] < rms[2]);
        CHECK(rms[2] - rms[0] >= 6.0);
        ++notes;
        ordered += (cen[0] < cen[1] && cen[1] < cen[2]);
        brassy += (hi[2] - hi[0] >= 6.0);
    }
    CHECK(ordered >= static_cast<int>(std::ceil(0.9 * notes)));
    CHECK(brassy >= static_cast<int>(std::ceil(0.8 * notes)));
}
