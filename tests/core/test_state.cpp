// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-011 parameter clamping and plugin state persistence (D-015, D-016; A-015 attack surface).
#include "tpt/State.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <limits>
#include <string>

TEST_CASE("T-011 VoiceParameters::clamped", "[T-011][unit]") {
    tpt::VoiceParameters p;
    p.brightness = 5.0f; p.lipStiffness = -1.0f; p.breathNoise = 2.0f; p.vibratoRateHz = 100.0f;
    p.vibratoDepth = -3.0f; p.tuningA4Hz = 300.0f; p.intonationRealism = 9.0f; p.outputGainDb = 40.0f; p.overblow = -7.0f;
    const auto c = p.clamped();
    CHECK(c.brightness == 1.0f); CHECK(c.lipStiffness == 0.0f); CHECK(c.breathNoise == 1.0f);
    CHECK(c.vibratoRateHz == 8.0f); CHECK(c.vibratoDepth == 0.0f); CHECK(c.tuningA4Hz == 415.0f);
    CHECK(c.intonationRealism == 1.0f); CHECK(c.outputGainDb == 12.0f); CHECK(c.overblow == -1.0f);
    tpt::VoiceParameters q;
    q.brightness = std::numeric_limits<float>::quiet_NaN();
    q.tuningA4Hz = std::numeric_limits<float>::infinity();
    const auto d = q.clamped();
    CHECK(d.brightness == tpt::VoiceParameters{}.brightness);
    CHECK(d.tuningA4Hz == tpt::VoiceParameters{}.tuningA4Hz);
}

TEST_CASE("T-011 state round trip", "[T-011][unit]") {
    tpt::VoiceParameters p;
    p.brightness = 0.8f; p.lipStiffness = 0.2f; p.breathNoise = 0.0f; p.vibratoRateHz = 6.5f; p.vibratoDepth = 0.4f;
    p.tuningA4Hz = 442.0f; p.intonationRealism = 1.0f; p.outputGainDb = -6.0f; p.overblow = 0.3f;
    p.useAlternates = true; p.fixedValves = true;
    const std::string s = tpt::serializeState(p);
    CHECK(s.find("\"format\":\"tpt-state-1\"") != std::string::npos);
    const auto r = tpt::deserializeState(s);
    REQUIRE(r.has_value());
    CHECK(r->brightness == p.brightness); CHECK(r->lipStiffness == p.lipStiffness); CHECK(r->breathNoise == p.breathNoise);
    CHECK(r->vibratoRateHz == p.vibratoRateHz); CHECK(r->vibratoDepth == p.vibratoDepth); CHECK(r->tuningA4Hz == p.tuningA4Hz);
    CHECK(r->intonationRealism == p.intonationRealism); CHECK(r->outputGainDb == p.outputGainDb); CHECK(r->overblow == p.overblow);
    CHECK(r->useAlternates); CHECK(r->fixedValves);
}

TEST_CASE("T-011 garbage and partial state", "[T-011][unit]") {
    for (const char* bad : {"", "null", "[]", "{", "{\"format\":\"tpt-state-2\",\"params\":{}}", "\xff\xfe", "{\"params\":{}}",
                            "{\"format\":\"tpt-state-1\",\"params\":[1,2]}"}) {
        INFO(bad);
        CHECK_FALSE(tpt::deserializeState(bad).has_value());
    }
    CHECK_FALSE(tpt::deserializeState(std::string(64 * 1024 + 1, ' ')).has_value());
    const auto r = tpt::deserializeState("{\"format\":\"tpt-state-1\",\"params\":{\"brightness\":\"x\",\"overblow\":3,"
                                         "\"useAlternates\":1,\"unknown\":true}}");
    REQUIRE(r.has_value());
    CHECK(r->brightness == tpt::VoiceParameters{}.brightness);   // wrong type -> default
    CHECK(r->overblow == 1.0f);                                  // clamped
    CHECK_FALSE(r->useAlternates);                               // number is not a boolean -> default
    std::string deep(2000, '[');
    CHECK_FALSE(tpt::deserializeState(deep).has_value());
}
