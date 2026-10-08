// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-003 fingering table == frozen fixture (D-002; C-057..C-065). T-004 note resolution (normal mode, alternates,
// range, extended). T-005 Fixed-valves resolution (D-003).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <set>

namespace {
tpt::Trigger trig(const std::string& t) {
    if (t == "-") return tpt::Trigger::None;
    if (t == "1") return tpt::Trigger::First;
    if (t == "3") return tpt::Trigger::Third;
    throw std::runtime_error("bad trigger " + t);
}
} // namespace

TEST_CASE("T-003 compiled table equals the fixture", "[T-003][unit]") {
    const auto fx = tpttest::loadFingeringFixture();
    const auto& tab = tpt::fingeringTable();
    REQUIRE(tab.size() == fx.size());
    for (size_t i = 0; i < fx.size(); ++i) {
        INFO("fixture row " << i << " written " << fx[i].written);
        CHECK(tab[i].written == fx[i].written);
        CHECK(tpt::valvesName(tab[i].valves) == fx[i].valves);
        CHECK(tab[i].partial == fx[i].partial);
        CHECK(tab[i].trigger == trig(fx[i].trigger));
        CHECK(tab[i].alternate == fx[i].alternate);
    }
}

TEST_CASE("T-003 standard and alternate lookups", "[T-003][unit]") {
    const auto fx = tpttest::loadFingeringFixture();
    std::set<int> std_written;
    for (const auto& f : fx) {
        const auto got = f.alternate ? tpt::alternateFingering(f.written) : tpt::standardFingering(f.written);
        REQUIRE(got.has_value());
        CHECK(tpt::valvesName(got->valves) == f.valves);
        CHECK(got->partial == f.partial);
        if (!f.alternate) std_written.insert(f.written);
    }
    // every written pitch of the range has exactly one standard fingering
    for (int w = tpt::kLowestWritten; w <= tpt::kHighestWritten; ++w) CHECK(std_written.count(w) == 1);
    CHECK(std_written.size() == static_cast<size_t>(tpt::kHighestWritten - tpt::kLowestWritten + 1));
    CHECK_FALSE(tpt::standardFingering(53).has_value());
    CHECK_FALSE(tpt::standardFingering(90).has_value());
    CHECK_FALSE(tpt::alternateFingering(60).has_value());   // C4 has no alternate in the table
    // the partial is consistent with the nominal pitch rule (C-056): round(48 + 12 log2 n - lowering) == written
    for (const auto& f : tpt::fingeringTable()) {
        const double nominal = 48.0 + 12.0 * std::log2(double(f.partial)) - tpt::loweringSemitones(f.valves);
        INFO("written " << f.written << " valves " << tpt::valvesName(f.valves) << " p" << f.partial);
        CHECK(std::lround(nominal) == f.written);
    }
}

TEST_CASE("T-004 normal-mode resolution", "[T-004][unit]") {
    for (int concert = 28; concert <= 100; ++concert) {
        const int w = concert + 2;
        const auto r = tpt::resolveNote(concert, false, std::nullopt);
        INFO("concert " << concert);
        if (w < tpt::kLowestWritten || w > tpt::kHighestWritten) { CHECK_FALSE(r.has_value()); continue; }
        REQUIRE(r.has_value());
        CHECK(r->concert == concert);
        CHECK(r->written == w);
        CHECK(r->fingering == *tpt::standardFingering(w));
        CHECK(r->extended == (w > tpt::kHighestNormalWritten));
        CHECK_FALSE(r->fixedValves);
        const auto ra = tpt::resolveNote(concert, true, std::nullopt);
        REQUIRE(ra.has_value());
        const auto alt = tpt::alternateFingering(w);
        CHECK(ra->fingering == (alt ? *alt : *tpt::standardFingering(w)));
    }
    CHECK_FALSE(tpt::resolveNote(27, false, std::nullopt).has_value());
    CHECK_FALSE(tpt::resolveNote(101, true, std::nullopt).has_value());
    // spot check: concert 65 = written G4: standard open p3, alternate 1-3 p4
    CHECK(tpt::valvesName(tpt::resolveNote(65, false, std::nullopt)->fingering.valves) == "0");
    CHECK(tpt::resolveNote(65, true, std::nullopt)->fingering.partial == 4);
    CHECK(tpt::valvesName(tpt::resolveNote(65, true, std::nullopt)->fingering.valves) == "13");
}

TEST_CASE("T-005 Fixed-valves resolution", "[T-005][unit]") {
    // Rule (D-003) re-implemented independently here
    for (tpt::Valves v = 0; v < 8; ++v) {
        for (int concert = 28; concert <= 100; ++concert) {
            const auto r = tpt::resolveNote(concert, false, v);
            REQUIRE(r.has_value());
            int bestN = 1; double bestD = 1e9, bestNom = 0;
            for (int n = 1; n <= tpt::kMaxPartial; ++n) {
                const double nom = 48.0 + 12.0 * std::log2(double(n)) - tpt::loweringSemitones(v);
                const double d = std::fabs(nom - (concert + 2));
                if (d < bestD - 1e-12) { bestD = d; bestN = n; bestNom = nom; }
            }
            INFO("valves " << int(v) << " concert " << concert);
            CHECK(r->fingering.valves == v);
            CHECK(r->fingering.partial == bestN);
            CHECK(r->written == static_cast<int>(std::lround(bestNom)));
            CHECK(r->concert == r->written - 2);
            CHECK(r->fingering.trigger == tpt::Trigger::None);
            CHECK_FALSE(r->fingering.alternate);
            CHECK(r->fixedValves);
        }
        CHECK_FALSE(tpt::resolveNote(27, false, v).has_value());
        CHECK_FALSE(tpt::resolveNote(101, false, v).has_value());
    }
    // spot checks computed by script (session 2026-10-08): (concert, valves) -> (partial, written)
    const auto a = tpt::resolveNote(60, false, *tpt::parseValves("13"));
    CHECK(a->fingering.partial == 3); CHECK(a->written == 62);
    const auto b = tpt::resolveNote(63, false, *tpt::parseValves("13"));
    CHECK(b->fingering.partial == 4); CHECK(b->written == 67);
    const auto c = tpt::resolveNote(40, false, *tpt::parseValves("123"));
    CHECK(c->fingering.partial == 1); CHECK(c->written == 42);
    const auto d = tpt::resolveNote(72, false, *tpt::parseValves("0"));
    CHECK(d->fingering.partial == 5); CHECK(d->written == 76);
}
