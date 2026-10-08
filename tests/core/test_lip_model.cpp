// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-008 lip settings and threshold pressures (D-005; Doc, Vergez & Hannebicq 2023 Tables I and II = C-036, C-053;
// Freour 2022 Table 1 = C-044 for the f_l / f_res ratios).
#include "tpt/LipModel.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <stdexcept>

using Catch::Approx;

TEST_CASE("T-008 lip setting per partial (D-005)", "[T-008][unit]") {
    // Doc 2023 Table II: f_l (Hz) and h0 (mm) for registers 2..6; Freour 2022 Table 1 resonance (Hz) of the same register
    const double docFl[] = {235.0, 340.0, 467.0, 586.0, 703.0};
    const double docH0mm[] = {0.242, 0.218, 0.190, 0.172, 0.168};
    const double freourF[] = {232.70, 348.07, 462.60, 582.14, 690.58};
    for (int i = 0; i < 5; ++i) {
        const int n = i + 2;
        const double fres = 300.0 + 37.0 * n;   // arbitrary resonance: the ratio must be applied to it
        const auto p = tpt::lipSettingFor(n, fres);
        INFO("partial " << n);
        CHECK(p.flHz == Approx(docFl[i] / freourF[i] * fres).epsilon(1e-12));
        CHECK(p.h0M == Approx(docH0mm[i] * 1e-3).epsilon(1e-12));
        CHECK(p.Ql == Approx(20.0));
        CHECK(p.mu == Approx(9.0));
        CHECK(p.widthM == Approx(12e-3));
    }
    // partial 1: ratio 1.01, h0 0.27 mm; partials 7..13: ratio of partial 6, h0 = max(0.12, 0.168 - 0.006 (n - 6)) mm
    CHECK(tpt::lipSettingFor(1, 100.0).flHz == Approx(101.0).epsilon(1e-12));
    CHECK(tpt::lipSettingFor(1, 100.0).h0M == Approx(0.27e-3).epsilon(1e-12));
    for (int n = 7; n <= 13; ++n) {
        const auto p = tpt::lipSettingFor(n, 1000.0);
        INFO("partial " << n);
        CHECK(p.flHz == Approx(703.0 / 690.58 * 1000.0).epsilon(1e-12));
        CHECK(p.h0M == Approx(std::max(0.12, 0.168 - 0.006 * (n - 6)) * 1e-3).epsilon(1e-12));
    }
    CHECK_THROWS_AS(tpt::lipSettingFor(0, 100.0), std::invalid_argument);
    CHECK_THROWS_AS(tpt::lipSettingFor(14, 100.0), std::invalid_argument);
    CHECK_THROWS_AS(tpt::lipSettingFor(3, 0.0), std::invalid_argument);
}

TEST_CASE("T-008 threshold pressures (Doc 2023 Table I, D-005)", "[T-008][unit]") {
    const double table1[] = {1047.0, 1788.0, 2477.0, 3158.0, 4109.0};
    for (int i = 0; i < 5; ++i) CHECK(tpt::thresholdPressurePa(i + 2) == Approx(table1[i]));
    CHECK(tpt::thresholdPressurePa(1) == Approx(800.0));
    for (int n = 7; n <= 13; ++n) CHECK(tpt::thresholdPressurePa(n) == Approx(4109.0 + 700.0 * (n - 6)));
    CHECK_THROWS_AS(tpt::thresholdPressurePa(0), std::invalid_argument);
    CHECK_THROWS_AS(tpt::thresholdPressurePa(14), std::invalid_argument);
}
