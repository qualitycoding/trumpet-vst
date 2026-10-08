// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-010 analysis utilities used by the other tests (D-017). Expected literals from expected_values.py.
#include "tpt/Analysis.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>

using Catch::Approx;

namespace {
std::vector<float> tone(double fs, double sec, std::vector<std::pair<double, double>> partials) {
    std::vector<float> x(static_cast<size_t>(fs * sec));
    for (size_t i = 0; i < x.size(); ++i) {
        double s = 0;
        for (auto [f, a] : partials) s += a * std::sin(2 * M_PI * f * double(i) / fs);
        x[i] = static_cast<float>(s);
    }
    return x;
}
} // namespace

TEST_CASE("T-010 YIN f0", "[T-010][unit]") {
    for (double f : {82.0, 233.08, 466.16, 1046.5, 1244.5}) {
        const auto x = tone(48000, 0.5, {{f, 0.5}, {2 * f, 0.3}, {3 * f, 0.2}});
        const double est = tpt::estimateF0(x.data(), x.size(), 48000);
        INFO("f " << f);
        CHECK(std::fabs(1200 * std::log2(est / f)) < 0.5);
    }
    // weak fundamental (pp high register is nearly a sine, ff low register has H1 well below H3, C-085 spectra)
    for (double f : {164.8, 233.08}) {
        const auto x = tone(48000, 0.5, {{f, 0.05}, {2 * f, 0.4}, {3 * f, 0.5}, {4 * f, 0.4}, {5 * f, 0.3}});
        INFO("weak fundamental f " << f);
        CHECK(std::fabs(1200 * std::log2(tpt::estimateF0(x.data(), x.size(), 48000) / f)) < 1.0);
    }
    std::vector<float> silence(24000, 0.0f);
    CHECK(tpt::estimateF0(silence.data(), silence.size(), 48000) == 0.0);
}

TEST_CASE("T-010 harmonic levels, centroid, partial, rms", "[T-010][unit]") {
    const auto x = tone(48000, 0.5, {{300.0, 1.0}, {600.0, 0.5}, {900.0, 0.25}});
    const auto h = tpt::harmonicLevelsDb(x.data(), x.size(), 48000, 300.0, 3);
    REQUIRE(h.size() == 3);
    CHECK(h[0] == Approx(0.0).margin(0.3));
    CHECK(h[1] == Approx(-6.020599913279624).margin(0.3));    // EXPECTED:db_half
    CHECK(h[2] == Approx(-12.041199826559248).margin(0.3));   // EXPECTED:db_quarter
    CHECK_THROWS_AS(tpt::harmonicLevelsDb(x.data(), x.size(), 48000, 0.0, 3), std::invalid_argument);
    CHECK_THROWS_AS(tpt::harmonicLevelsDb(x.data(), 0, 48000, 300.0, 3), std::invalid_argument);

    const auto s = tone(48000, 0.5, {{1000.0, 1.0}});
    CHECK(tpt::spectralCentroidHz(s.data(), s.size(), 48000) == Approx(1000.0).epsilon(0.01));
    CHECK(tpt::rmsDb(s.data(), s.size()) == Approx(-3.0102999566398125).margin(0.01));   // EXPECTED:sine_rms_db
    std::vector<float> z(100, 0.0f);
    CHECK(tpt::rmsDb(z.data(), z.size()) == -200.0);
    CHECK(tpt::spectralCentroidHz(z.data(), z.size(), 48000) == 0.0);

    const std::vector<double> res = {100.0, 200.0, 300.0, 400.0};
    CHECK(tpt::nearestPartial(205.0, res) == 2);
    CHECK(tpt::nearestPartial(345.0, res) == 3);   // log scale: 345 is nearer 300 than 400
    CHECK(tpt::nearestPartial(10.0, res) == 1);
    CHECK(tpt::nearestPartial(0.0, res) == 0);
    CHECK(tpt::nearestPartial(200.0, {}) == 0);
}
