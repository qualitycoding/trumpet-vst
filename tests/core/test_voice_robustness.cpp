// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-023 operational robustness: parameter extremes, sample rates, block sizes, determinism, release to digital
// silence, invalid input (SC-9, D-006). T-025 latency reporting (A-012, D-006).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <limits>

using namespace tpttest;

namespace {
float peakAbs(const std::vector<float>& x) { float m = 0; for (float s : x) m = std::max(m, std::fabs(s)); return m; }
} // namespace

TEST_CASE("T-023 parameter extremes stay finite and bounded", "[T-023][operational]") {
    const double fs = 48000;
    for (int mask = 0; mask < 64; ++mask) {
        tpt::VoiceParameters p;
        p.brightness = (mask & 1) ? 1.0f : 0.0f;
        p.lipStiffness = (mask & 2) ? 1.0f : 0.0f;
        p.breathNoise = (mask & 4) ? 1.0f : 0.0f;
        p.vibratoDepth = (mask & 8) ? 1.0f : 0.0f;
        p.outputGainDb = (mask & 16) ? 12.0f : -24.0f;
        p.overblow = (mask & 32) ? 1.0f : -1.0f;
        p.intonationRealism = (mask & 1) ? 0.0f : 1.0f;
        for (int concert : {52, 68, 82}) {
            const auto x = renderNote(fs, concert, kVelFF, 0.6, p);
            INFO("mask " << mask << " concert " << concert);
            REQUIRE(allFinite(x));
            CHECK(peakAbs(x) <= 1.0f);
        }
    }
}

TEST_CASE("T-023 sample rates and latency (T-025)", "[T-023][T-025][operational]") {
    for (double fs : {22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0}) {
        auto v = makeVoice(fs, 512);
        const int lat = v->latencySamples();
        INFO("fs " << fs);
        CHECK(lat >= 0);
        CHECK(lat <= 64);
        if (fs >= 88200.0) CHECK(lat == 0);   // no oversampling at >= 88.2 kHz (D-006)
        v->noteOn(67, kVelMF);
        const auto x = render(*v, fs, 1.0, 512);
        REQUIRE(allFinite(x));
        CHECK(v->latencySamples() == lat);
        const double f0 = f0Of(tail(x, fs, 0.3), fs);
        CHECK(std::fabs(1200 * std::log2(f0 / tpt::equalTemperedHz(67))) <= 15.0);
    }
}

TEST_CASE("T-023 output is independent of the host block size and deterministic", "[T-023][operational]") {
    const double fs = 48000;
    tpt::VoiceParameters p; p.breathNoise = 0.5f; p.vibratoDepth = 0.3f;
    const auto ref = renderNote(fs, 64, kVelMF, 0.5, p, 256);
    CHECK(ref == renderNote(fs, 64, kVelMF, 0.5, p, 256));   // deterministic (seeded noise, D-012)
    for (int block : {1, 7, 64, 1000, 4096}) {
        INFO("block " << block);
        CHECK(renderNote(fs, 64, kVelMF, 0.5, p, block) == ref);
    }
}

TEST_CASE("T-023 release to digital silence and invalid input", "[T-023][operational]") {
    const double fs = 48000;
    auto v = makeVoice(fs);
    v->noteOn(70, kVelFF);
    render(*v, fs, 0.5);
    v->noteOff(70);
    const auto x = render(*v, fs, 0.8);
    const auto last = tail(x, fs, 0.1);
    CHECK(std::all_of(last.begin(), last.end(), [](float s) { return s == 0.0f; }));
    tpt::VoiceParameters bad;
    bad.brightness = std::numeric_limits<float>::quiet_NaN();
    bad.overblow = std::numeric_limits<float>::infinity();
    v->setParameters(bad);
    v->setPitchBend(std::numeric_limits<float>::quiet_NaN());
    v->setBreath(std::numeric_limits<float>::infinity());
    v->noteOn(-5, 0.5f);
    v->noteOn(500, 0.5f);
    v->noteOn(67, std::numeric_limits<float>::quiet_NaN());
    v->noteOn(67, kVelMF);
    const auto y = render(*v, fs, 0.5);
    CHECK(allFinite(y));
    CHECK(peakAbs(y) <= 1.0f);
    CHECK_THROWS_AS(v->prepare(8000.0, 256), std::invalid_argument);
    CHECK_THROWS_AS(v->prepare(48000.0, 0), std::invalid_argument);
    CHECK_THROWS_AS(tpt::TrumpetVoice(nullptr), std::invalid_argument);
}

TEST_CASE("T-023 long random performance", "[T-023][operational]") {
    const double fs = 48000;
    auto v = makeVoice(fs, 128);
    uint32_t seed = 20261008u;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    std::vector<float> buf(128);
    int held = -1;
    for (int b = 0; b < static_cast<int>(30.0 * fs / 128); ++b) {
        if (b % 40 == 0) {
            if (held >= 0 && rnd() % 2) v->noteOff(held);
            held = 52 + static_cast<int>(rnd() % 36);
            v->noteOn(held, 0.1f + static_cast<float>(rnd() % 90) / 100.0f);
            tpt::VoiceParameters p; p.overblow = static_cast<float>(static_cast<int>(rnd() % 21) - 10) / 10.0f;
            p.fixedValves = (rnd() % 7) == 0;
            v->setParameters(p);
            v->keyswitch(24 + static_cast<int>(rnd() % 8));
            v->setPitchBend(static_cast<float>(static_cast<int>(rnd() % 41) - 20) / 10.0f);
        }
        v->process(buf.data(), 128);
        for (float s : buf) { REQUIRE(std::isfinite(s)); REQUIRE(std::fabs(s) <= 1.0f); }
    }
}
