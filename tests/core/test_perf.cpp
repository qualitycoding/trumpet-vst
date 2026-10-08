// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-024b real-time factor <= 0.05 at 48 kHz / 128-sample blocks (A-012, SC-7). Release builds only (ctest label perf).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>

TEST_CASE("T-024 real-time factor", "[T-024][perf]") {
    const double fs = 48000;
    auto v = tpttest::makeVoice(fs, 128);
    std::vector<float> buf(128);
    const int blocks = static_cast<int>(20.0 * fs / 128);
    const auto t0 = std::chrono::steady_clock::now();
    tpt::VoiceParameters p; p.breathNoise = 0.3f; p.vibratoDepth = 0.5f;
    for (int b = 0; b < blocks; ++b) {
        if (b % 300 == 0) { v->noteOn(52 + (b / 300) % 36, 0.9f); p.overblow = (b / 300) % 3 == 0 ? 0.6f : 0.0f; v->setParameters(p); }
        v->process(buf.data(), 128);
    }
    const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    const double rtf = secs / 20.0;
    INFO("real-time factor " << rtf);
    CHECK(rtf <= 0.05);
}
