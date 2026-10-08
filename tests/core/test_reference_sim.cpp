// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-009 reference integrator reproduces Freour et al. 2022 (C-044, C-075) and the Doc 2023 register selection on the
// embedded table (C-080). D-006.
#include "TestSupport.h"
#include "tpt/LipModel.h"
#include "tpt/ReferenceSim.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

namespace {
std::vector<tpt::Mode> freourModes() {
    std::vector<tpt::Mode> m;
    for (const auto& x : tpttest::loadMeasuredModes()) m.push_back({{x.re, x.im}, {x.cre, x.cim}});
    return m;
}
tpt::RefSimConfig freourConfig(double pm) {
    tpt::RefSimConfig c;
    c.modes = freourModes();
    c.lip = {382.18, 3.0, 2.0, 8e-3, 0.1e-3};   // Freour 2022 Table 2, f_l from p. 6
    c.pmPa = pm;
    c.durationS = 2.0;
    c.sampleRate = 48000.0;
    c.oversample = 2;
    return c;
}
double rmsLast(const std::vector<double>& x, double fs, double sec) {
    const size_t n = static_cast<size_t>(sec * fs);
    double m = 0, e = 0;
    for (size_t i = x.size() - n; i < x.size(); ++i) m += x[i];
    m /= double(n);
    for (size_t i = x.size() - n; i < x.size(); ++i) e += (x[i] - m) * (x[i] - m);
    return std::sqrt(e / double(n));
}
std::vector<float> lastAsFloat(const std::vector<double>& x, double fs, double sec) {
    const size_t n = static_cast<size_t>(sec * fs);
    return std::vector<float>(x.end() - static_cast<long>(n), x.end());
}
} // namespace

TEST_CASE("T-009 Freour 2022 B-flat4 reproduction", "[T-009][integration]") {
    for (double pm : {3000.0, 5000.0}) {
        const auto p = tpt::simulateReference(freourConfig(pm));
        REQUIRE(p.size() == 96000);
        INFO("pm " << pm);
        CHECK(rmsLast(p, 48000, 0.5) > 500.0);
        const auto seg = lastAsFloat(p, 48000, 0.5);
        const double f0 = tpttest::f0Of(seg, 48000);
        CHECK(f0 >= 465.0);
        CHECK(f0 <= 490.0);
    }
    // below the Hopf point the equilibrium is stable: no oscillation from rest
    CHECK(rmsLast(tpt::simulateReference(freourConfig(2000.0)), 48000, 0.5) < 10.0);
}

TEST_CASE("T-009 deterministic and argument checks", "[T-009][integration]") {
    auto c = freourConfig(4000.0);
    c.durationS = 0.3;
    CHECK(tpt::simulateReference(c) == tpt::simulateReference(c));
    auto bad = c; bad.modes.clear();
    CHECK_THROWS_AS(tpt::simulateReference(bad), std::invalid_argument);
    bad = c; bad.sampleRate = 0;
    CHECK_THROWS_AS(tpt::simulateReference(bad), std::invalid_argument);
    bad = c; bad.durationS = -1;
    CHECK_THROWS_AS(tpt::simulateReference(bad), std::invalid_argument);
}

TEST_CASE("T-009 Doc 2023 lip settings select partials 2..6 on the embedded open state", "[T-009][integration]") {
    const auto t = tpttest::embeddedTable();
    const size_t open = t->stateFor(0);
    std::vector<double> res;
    for (int k = 1; k <= 13; ++k) res.push_back(t->partialHz(open, k));
    for (int n = 2; n <= 6; ++n) {
        tpt::RefSimConfig c;
        c.modes = t->state(open).modes;
        c.lip = tpt::lipSettingFor(n, t->partialHz(open, n));
        c.pmPa = 2.5 * tpt::thresholdPressurePa(n);
        c.attackS = 0.003;
        c.yInitM = 0.0;
        c.durationS = 0.8;
        const auto p = tpt::simulateReference(c);
        const auto seg = lastAsFloat(p, c.sampleRate, 0.3);
        INFO("partial " << n);
        CHECK(tpt::nearestPartial(tpttest::f0Of(seg, c.sampleRate), res) == n);
    }
}
