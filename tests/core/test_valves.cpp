// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-002 Valve combinations and keyswitches (D-002, D-003; C-067).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("T-002 lowering, names, parsing", "[T-002][unit]") {
    using namespace tpt;
    CHECK(loweringSemitones(0) == 0);
    CHECK(loweringSemitones(kValve2) == 1);
    CHECK(loweringSemitones(kValve1) == 2);
    CHECK(loweringSemitones(kValve3) == 3);
    CHECK(loweringSemitones(kValve1 | kValve2) == 3);
    CHECK(loweringSemitones(kValve2 | kValve3) == 4);
    CHECK(loweringSemitones(kValve1 | kValve3) == 5);
    CHECK(loweringSemitones(kValve1 | kValve2 | kValve3) == 6);
    CHECK_THROWS_AS(loweringSemitones(8), std::out_of_range);

    const char* names[8] = {"0", "1", "2", "12", "3", "13", "23", "123"};   // index = mask
    for (Valves m = 0; m < 8; ++m) {
        CHECK(valvesName(m) == names[m]);
        REQUIRE(parseValves(names[m]).has_value());
        CHECK(*parseValves(names[m]) == m);
    }
    CHECK_THROWS_AS(valvesName(8), std::out_of_range);
    for (const char* bad : {"", "21", "4", "1 2", "o", "0 ", "1-3", "1233"}) CHECK_FALSE(parseValves(bad).has_value());
}

TEST_CASE("T-002 keyswitch notes 24..31", "[T-002][unit]") {
    using namespace tpt;
    const char* order[8] = {"0", "2", "1", "12", "23", "13", "123", "3"};
    for (int k = 0; k < 8; ++k) {
        const auto v = keyswitchValves(kKeyswitchLow + k);
        REQUIRE(v.has_value());
        CHECK(valvesName(*v) == order[k]);
    }
    CHECK(kKeyswitchHigh == 31);
    CHECK_FALSE(keyswitchValves(23).has_value());
    CHECK_FALSE(keyswitchValves(32).has_value());
    CHECK_FALSE(keyswitchValves(60).has_value());
}
