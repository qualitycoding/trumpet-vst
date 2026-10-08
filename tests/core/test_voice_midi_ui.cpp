// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-019 MIDI behaviour and expression (A-010, D-014; SC-6). T-020 UI state per note (A-009, D-013; SC-2).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>

using namespace tpttest;

namespace {
double cents(double f, double ref) { return 1200.0 * std::log2(f / ref); }
double rmsOf(const std::vector<float>& x) { return tpt::rmsDb(x.data(), x.size()); }
} // namespace

TEST_CASE("T-019 monophonic last-note priority and release", "[T-019][integration]") {
    const double fs = 48000;
    auto v = makeVoice(fs);
    v->noteOn(58, kVelMF);
    render(*v, fs, 0.4);
    v->noteOn(65, kVelMF);
    auto x = render(*v, fs, 0.4);
    CHECK(std::fabs(cents(f0Of(tail(x, fs, 0.2), fs), tpt::equalTemperedHz(65))) <= 15.0);
    v->noteOff(65);                                     // falls back to the still-held 58
    x = render(*v, fs, 0.4);
    CHECK(std::fabs(cents(f0Of(tail(x, fs, 0.2), fs), tpt::equalTemperedHz(58))) <= 15.0);
    v->noteOff(58);
    x = render(*v, fs, 0.6);
    CHECK(rmsOf(tail(x, fs, 0.2)) < -60.0);
    CHECK_FALSE(v->uiState().sounding);
    CHECK(v->uiState().concert == 0);
    v->noteOn(60, kVelMF);
    render(*v, fs, 0.3);
    v->noteOn(60, 0.0f);                                // velocity 0 = note off
    x = render(*v, fs, 0.6);
    CHECK(rmsOf(tail(x, fs, 0.2)) < -60.0);
    v->noteOn(60, kVelMF);
    render(*v, fs, 0.3);
    v->allNotesOff();
    x = render(*v, fs, 0.6);
    CHECK(rmsOf(tail(x, fs, 0.2)) < -60.0);
    v->noteOn(30, kVelMF);                              // out of range (written 32): ignored
    x = render(*v, fs, 0.3);
    CHECK(rmsOf(x) < -120.0);
}

TEST_CASE("T-019 pitch bend, vibrato", "[T-019][integration]") {
    const double fs = 48000;
    for (float bend : {2.0f, -1.0f}) {
        auto v = makeVoice(fs);
        v->noteOn(67, kVelMF);
        render(*v, fs, 0.5);
        const double f0 = f0Of(render(*v, fs, 0.2), fs);
        v->setPitchBend(bend);
        const auto x = render(*v, fs, 0.5);
        INFO("bend " << bend);
        CHECK(std::fabs(cents(f0Of(tail(x, fs, 0.2), fs), f0) - 100.0 * bend) <= 15.0);
    }
    auto v = makeVoice(fs);
    v->noteOn(67, kVelMF);
    render(*v, fs, 0.5);
    v->setVibratoControl(1.0f);
    const auto x = render(*v, fs, 1.0);
    double lo = 1e9, hi = -1e9;
    for (size_t k = static_cast<size_t>(0.3 * fs); k + 960 <= x.size(); k += 480) {
        const double c = cents(tpt::estimateF0(x.data() + k, 960, fs, 40.0, 2500.0), tpt::equalTemperedHz(67));
        lo = std::min(lo, c); hi = std::max(hi, c);
    }
    CHECK(hi - lo >= 35.0);   // depth 1 = +-25 cents -> 50 cents peak to peak (frame averaging allowed)
    CHECK(hi - lo <= 65.0);
}

TEST_CASE("T-019 breath controller replaces velocity", "[T-019][integration]") {
    const double fs = 48000;
    auto levelAt = [&](float vel, float breath) {
        auto v = makeVoice(fs);
        v->setBreath(breath);
        v->noteOn(67, vel);
        const auto x = render(*v, fs, 0.8);
        const auto t = tail(x, fs, 0.3);
        return std::pair<double, double>{tpt::rmsDb(t.data(), t.size()), tpt::spectralCentroidHz(t.data(), t.size(), fs)};
    };
    const auto soft = levelAt(0.6f, 0.2f), loud = levelAt(0.6f, 0.95f);
    CHECK(loud.first - soft.first >= 6.0);
    CHECK(loud.second > soft.second);
    const auto a = levelAt(0.2f, 0.6f), b = levelAt(1.0f, 0.6f);
    CHECK(std::fabs(a.first - b.first) <= 1.5);         // once breath is present, velocity no longer sets the level
}

TEST_CASE("T-020 UI state for every fingering", "[T-020][integration]") {
    const double fs = 48000;
    for (float realism : {0.0f, 1.0f}) {
        tpt::VoiceParameters p; p.intonationRealism = realism;
        for (const auto& f : tpt::fingeringTable()) {
            if (f.alternate || f.written >= 88) continue;
            auto v = makeVoice(fs, 256, p);
            v->noteOn(f.written - 2, kVelMF);
            render(*v, fs, 0.5);
            const auto s = v->uiState();
            INFO("written " << f.written << " realism " << realism);
            CHECK(s.sounding);
            CHECK(s.concert == f.written - 2);
            CHECK(s.written == f.written);
            CHECK(s.valves == f.valves);
            CHECK(s.targetPartial == f.partial);
            CHECK(s.soundingPartial == f.partial);
            CHECK(s.extended == (f.written > tpt::kHighestNormalWritten));
            CHECK_FALSE(s.fixedValves);
            CHECK(s.trigger == (realism < 0.5f ? f.trigger : tpt::Trigger::None));
            // the snapshot is current within one block after a change
            v->noteOff(f.written - 2);
            render(*v, fs, 0.6);
            CHECK_FALSE(v->uiState().sounding);
        }
    }
}
