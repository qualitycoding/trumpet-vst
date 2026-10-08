// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-012 every fingering locks to its partial and sustains (SC-5; C-080, C-091, lesson L-20261007T150700Z).
// T-013 pitch within +-10 cents of 12-TET (SC-3; C-081). T-014 Intonation realism and tuning (A-006, D-011).
// T-018 alternate fingerings (A-007d).
#include "TestSupport.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using Catch::Approx;
using namespace tpttest;

namespace {
struct Played { double f0; tpt::UiState ui; bool sustained; };
/// Renders 2 s; "sustained" = rms(last 0.3 s) >= rms(0.5..0.8 s) - 2 dB (no ring-down; R5 review defect D-1).
Played play(double fs, int concert, float vel, const tpt::VoiceParameters& p = {}, double sec = 2.0) {
    auto v = makeVoice(fs, 256, p);
    v->noteOn(concert, vel);
    const auto x = render(*v, fs, sec);
    const size_t a = static_cast<size_t>(0.5 * fs), b = static_cast<size_t>(0.3 * fs);
    const double early = tpt::rmsDb(x.data() + a, b), late = tpt::rmsDb(x.data() + x.size() - b, b);
    return {f0Of(tail(x, fs, 0.3), fs), v->uiState(), late >= early - 2.0 && late > -60.0};
}
double cents(double f, double ref) { return 1200.0 * std::log2(f / ref); }
} // namespace

TEST_CASE("T-012 regime lock for every standard fingering at pp, mf, ff", "[T-012][integration]") {
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate) continue;
        const int concert = f.written - 2;
        // extended notes 86..89 are played with the D-023 blowing floor whatever the velocity
        for (float vel : {kVelPP, kVelMF, kVelFF}) {
            const auto r = play(48000, concert, vel);
            INFO("written " << f.written << " vel " << vel << " f0 " << r.f0);
            CHECK(r.sustained);
            CHECK(nominalPartialOf(r.f0, f.valves) == f.partial);
            CHECK(r.ui.soundingPartial == f.partial);
            CHECK(r.ui.targetPartial == f.partial);
        }
    }
}

TEST_CASE("T-012 regime lock at lip-stiffness extremes (mf, normal range)", "[T-012][integration]") {
    for (float s : {0.0f, 1.0f}) {
        tpt::VoiceParameters p; p.lipStiffness = s;
        for (const auto& f : tpt::fingeringTable()) {
            if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
            const auto r = play(48000, f.written - 2, kVelMF, p);
            INFO("stiffness " << s << " written " << f.written);
            CHECK(r.sustained);
            CHECK(nominalPartialOf(r.f0, f.valves) == f.partial);
        }
    }
}

TEST_CASE("T-013 tuning within 10 cents (mf at 44.1/48/96 kHz; pp and ff at 48 kHz)", "[T-013][integration]") {
    for (double fs : {44100.0, 48000.0, 96000.0}) {
        for (const auto& f : tpt::fingeringTable()) {
            if (f.alternate || f.written > 87) continue;
            const int concert = f.written - 2;
            const auto r = play(fs, concert, kVelMF);
            INFO("fs " << fs << " written " << f.written << " f0 " << r.f0);
            CHECK(std::fabs(cents(r.f0, tpt::equalTemperedHz(concert))) <= 10.0);
        }
    }
    for (float vel : {kVelPP, kVelFF}) {
        for (const auto& f : tpt::fingeringTable()) {
            if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
            const int concert = f.written - 2;
            const auto r = play(48000, concert, vel);
            INFO("vel " << vel << " written " << f.written << " f0 " << r.f0);
            CHECK(std::fabs(cents(r.f0, tpt::equalTemperedHz(concert))) <= 10.0);
        }
    }
}

TEST_CASE("T-014 Intonation realism = 1 reproduces the natural tendencies; A4 tuning", "[T-014][integration]") {
    const auto table = embeddedTable();
    tpt::VoiceParameters p; p.intonationRealism = 1.0f;
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
        const int concert = f.written - 2;
        const auto* e = table->note(f.written, false);
        REQUIRE(e != nullptr);
        const auto r = play(48000, concert, kVelMF, p);
        INFO("written " << f.written << " natural " << e->naturalDevCents << " f0 " << r.f0);
        CHECK(std::fabs(cents(r.f0, tpt::equalTemperedHz(concert)) - e->naturalDevCents) <= 10.0);
    }
    tpt::VoiceParameters a; a.tuningA4Hz = 442.0f;
    for (int concert : {58, 67, 76}) {
        const double f440 = play(48000, concert, kVelMF).f0, f442 = play(48000, concert, kVelMF, a).f0;
        INFO("concert " << concert);
        CHECK(cents(f442, f440) == Approx(7.851415040126504).margin(3.0));   // EXPECTED:cents_442_440
    }
}

TEST_CASE("T-018 alternate fingerings", "[T-018][integration]") {
    tpt::VoiceParameters p; p.useAlternates = true;
    for (const auto& f : tpt::fingeringTable()) {
        if (!f.alternate) continue;
        const int concert = f.written - 2;
        const auto r = play(48000, concert, kVelMF, p);
        INFO("alt written " << f.written << " valves " << tpt::valvesName(f.valves) << " f0 " << r.f0);
        CHECK(r.sustained);
        CHECK(r.ui.valves == f.valves);
        CHECK(r.ui.targetPartial == f.partial);
        CHECK(nominalPartialOf(r.f0, f.valves) == f.partial);
        CHECK(std::fabs(cents(r.f0, tpt::equalTemperedHz(concert))) <= 10.0);
    }
}
