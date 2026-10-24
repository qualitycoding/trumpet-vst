// SPDX-License-Identifier: Apache-2.0
// Non-frozen self-test of the frozen helpers (S-001 3b): they must agree with their fixtures.
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>

using namespace tpttest;

TEST_CASE("support: fixtures load with the expected row counts", "[selftest]") {
    CHECK(loadMeasuredModes().size() == 11);
    CHECK(loadFingeringFixture().size() == 56);
}

TEST_CASE("support: nominalPartialOf inverts nominalPartialHz for every valve combination", "[selftest]") {
    for (int v = 0; v < 8; ++v)
        for (int m = 1; m <= 13; ++m) {
            INFO("valves " << v << " partial " << m);
            CHECK(nominalPartialOf(nominalPartialHz(static_cast<tpt::Valves>(v), m), static_cast<tpt::Valves>(v)) == m);
        }
    CHECK(nominalPartialOf(0, 0) == 0);
}
