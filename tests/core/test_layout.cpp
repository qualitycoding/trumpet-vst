// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-033 trumpet drawing geometry and caption text (D-013, A-009).
#include "tpt/Layout.h"
#include "tpt/Valves.h"
#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("T-033 layout rectangles", "[T-033][unit]") {
    const auto& L = tpt::trumpetLayout();
    std::vector<tpt::Rect> all;
    for (const auto& r : L.valveCap) all.push_back(r);
    for (const auto& r : L.valveCasing) all.push_back(r);
    for (const auto& r : L.ladder) all.push_back(r);
    all.push_back(L.firstTrigger); all.push_back(L.thirdTrigger); all.push_back(L.caption); all.push_back(L.instrument);
    for (const auto& r : all) CHECK(r.inside01());
    for (int i = 0; i < 3; ++i) {
        for (int j = i + 1; j < 3; ++j) { CHECK_FALSE(L.valveCap[i].intersects(L.valveCap[j])); CHECK_FALSE(L.valveCasing[i].intersects(L.valveCasing[j])); }
        CHECK_FALSE(L.valveCap[i].intersects(L.valveCasing[i]));
        CHECK(L.valveCap[i].y < L.valveCasing[i].y);
        if (i > 0) CHECK(L.valveCap[i].x > L.valveCap[i - 1].x);
    }
    for (size_t k = 0; k < L.ladder.size(); ++k) {
        for (size_t m = k + 1; m < L.ladder.size(); ++m) CHECK_FALSE(L.ladder[k].intersects(L.ladder[m]));
        if (k > 0) CHECK(L.ladder[k].y < L.ladder[k - 1].y);   // partial 1 at the bottom
        CHECK_FALSE(L.ladder[k].intersects(L.instrument));
        CHECK_FALSE(L.ladder[k].intersects(L.caption));
    }
    CHECK_FALSE(L.caption.intersects(L.instrument));
    CHECK_FALSE(L.firstTrigger.intersects(L.thirdTrigger));
    for (const auto& c : L.valveCap) { CHECK_FALSE(c.intersects(L.firstTrigger)); CHECK_FALSE(c.intersects(L.thirdTrigger)); }
}

TEST_CASE("T-033 caption text", "[T-033][unit]") {
    const std::string dot = " \xc2\xb7 ";
    tpt::UiState s;
    CHECK(tpt::captionText(s).empty());
    s.fixedValves = true; s.valves = tpt::kValve1 | tpt::kValve2;
    CHECK(tpt::captionText(s) == "Fixed valves 1-2");
    s = {};
    s.sounding = true; s.concert = 60; s.written = 62; s.valves = tpt::kValve1 | tpt::kValve3; s.trigger = tpt::Trigger::Third;
    s.targetPartial = 3; s.soundingPartial = 3;
    CHECK(tpt::captionText(s) == "Written D4" + dot + "Concert C4" + dot + "Valves 1-3" + dot + "Partial 3" + dot + "3rd slide");
    s.trigger = tpt::Trigger::None; s.soundingPartial = 4; s.targetPartial = 4; s.valves = tpt::kValve1 | tpt::kValve3;
    s.soundingPartial = 5;
    CHECK(tpt::captionText(s) == "Written D4" + dot + "Concert C4" + dot + "Valves 1-3" + dot + "Partial 4 (sounding 5)");
    s = {};
    s.sounding = true; s.concert = 86; s.written = 88; s.valves = 0; s.targetPartial = 10; s.soundingPartial = 10; s.extended = true;
    CHECK(tpt::captionText(s) == "Written E6" + dot + "Concert D6" + dot + "Valves open" + dot + "Partial 10" + dot + "extended");
    s = {};
    s.sounding = true; s.concert = 75; s.written = 77; s.valves = tpt::kValve1; s.trigger = tpt::Trigger::First;
    s.targetPartial = 6; s.soundingPartial = 0; s.fixedValves = true;
    CHECK(tpt::captionText(s) == "Written F5" + dot + "Concert D#5" + dot + "Valves 1" + dot + "Partial 6" + dot + "1st slide" + dot + "fixed valves");
}
