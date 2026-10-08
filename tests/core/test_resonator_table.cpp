// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-006 resonator table parsing and validation (D-004, A-015 attack surface).
// T-007 embedded table vs measurement and design rules (D-004, D-011; C-044, C-076, C-077, C-079).
#include "TestSupport.h"
#include "tpt/Errors.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <cmath>
#include <functional>

using Catch::Approx;
using nlohmann::json;

namespace {
json validFixture() { return json::parse(tpttest::readFixture("resonators_valid.json")); }

void expectRejected(const std::function<void(json&)>& mutate, const char* what) {
    json j = validFixture();
    mutate(j);
    INFO(what);
    CHECK_THROWS_AS(tpt::ResonatorTable::fromJson(j.dump()), tpt::ParseError);
}
} // namespace

TEST_CASE("T-006 valid fixture parses", "[T-006][unit]") {
    const json j = validFixture();
    const auto t = tpt::ResonatorTable::fromJson(j.dump());
    REQUIRE(t.stateCount() == j["states"].size());
    for (size_t i = 0; i < t.stateCount(); ++i) {
        CHECK(t.state(i).id == j["states"][i]["id"].get<std::string>());
        REQUIRE(t.state(i).modes.size() == j["states"][i]["modes"].size());
        for (size_t k = 0; k < t.state(i).modes.size(); ++k) {
            CHECK(t.state(i).modes[k].s.real() == j["states"][i]["modes"][k]["s"][0].get<double>());
            CHECK(t.state(i).modes[k].s.imag() == j["states"][i]["modes"][k]["s"][1].get<double>());
            CHECK(t.state(i).modes[k].R.real() == j["states"][i]["modes"][k]["R"][0].get<double>());
            CHECK(t.partialHz(i, static_cast<int>(k) + 1) ==
                  Approx(j["states"][i]["modes"][k]["s"][1].get<double>() / (2 * M_PI)).epsilon(1e-12));
        }
    }
    CHECK(t.state(t.stateFor(*tpt::parseValves("13"))).id == "13");
    CHECK(t.state(t.stateFor(0)).id == "0");
    const auto* n62 = t.note(62, false);
    REQUIRE(n62 != nullptr);
    CHECK(t.state(static_cast<size_t>(n62->stateIndex)).id == "13+t3@62");
    CHECK(t.state(static_cast<size_t>(n62->stateIndex)).trigger == tpt::Trigger::Third);
    CHECK(t.state(static_cast<size_t>(n62->stateIndex)).triggerLengthM == Approx(0.02));
    CHECK(n62->partial == 3);
    CHECK(n62->fscale == Approx(0.97));
    CHECK(n62->naturalDevCents == Approx(29.2));
    REQUIRE(t.note(67, true) != nullptr);
    CHECK(t.note(67, false) == nullptr);
    CHECK(t.note(61, false) == nullptr);
    CHECK(t.notes().size() == 3);
    REQUIRE(t.radiation().size() == 2);
    CHECK(t.radiation()[0].type == tpt::RadiationSection::Type::HighPass);
    CHECK(t.radiation()[1].type == tpt::RadiationSection::Type::Peak);
    CHECK(t.radiation()[1].gainDb == Approx(-3.0));
    CHECK(t.nlpLengthM() == Approx(0.85));
    CHECK(t.nlpBeta() == Approx(1.2));
    CHECK(t.airDensity() == Approx(1.176));
    CHECK(t.soundSpeed() == Approx(347.2));
    CHECK_THROWS_AS(t.state(t.stateCount()), std::out_of_range);
    CHECK_THROWS_AS(t.partialHz(0, 0), std::out_of_range);
    CHECK_THROWS_AS(t.partialHz(0, 5), std::out_of_range);
}

TEST_CASE("T-006 malformed tables are rejected with ParseError", "[T-006][unit]") {
    CHECK_THROWS_AS(tpt::ResonatorTable::fromJson(""), tpt::ParseError);
    CHECK_THROWS_AS(tpt::ResonatorTable::fromJson("{"), tpt::ParseError);
    CHECK_THROWS_AS(tpt::ResonatorTable::fromJson("[]"), tpt::ParseError);
    CHECK_THROWS_AS(tpt::ResonatorTable::fromJson(std::string(1024 * 1024 + 1, ' ')), tpt::ParseError);
    {
        std::string big = validFixture().dump();
        big.append(1024 * 1024 + 1 - big.size() + 10, ' ');
        CHECK_THROWS_AS(tpt::ResonatorTable::fromJson(big), tpt::ParseError);   // size limit before parsing
    }
    expectRejected([](json& j) { j.erase("format"); }, "missing format");
    expectRejected([](json& j) { j["format"] = "tpt-resonators-2"; }, "wrong format");
    expectRejected([](json& j) { j["states"] = json::array(); }, "no states");
    expectRejected([](json& j) { j["states"].erase(j["states"].begin() + 7); }, "missing valve combination 123");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["s"][0] = 1.0; }, "unstable pole Re(s) >= 0");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["s"][1] = -5.0; }, "negative Im(s)");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["s"][1] = 2 * M_PI * 25000.0; }, "pole above 20 kHz");
    expectRejected([](json& j) { j["states"][0]["modes"].erase(j["states"][0]["modes"].begin()); }, "fewer than 4 modes");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["R"][0] = 1e14; }, "residue too large");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["R"][0] = -1.0e9; }, "Re(R) <= 0 (flow solve needs Z0 > 0)");
    expectRejected([](json& j) { j["states"][0]["modes"][0]["R"] = json::array({1.0}); }, "residue not a pair");
    expectRejected([](json& j) { j["states"][1]["valves"] = "4"; }, "bad valves name");
    expectRejected([](json& j) { j["states"][8]["trigger"] = "2"; }, "bad trigger");
    expectRejected([](json& j) { j["states"][2]["id"] = j["states"][1]["id"]; }, "duplicate state id");
    expectRejected([](json& j) { j["notes"][0]["state"] = "nope"; }, "unknown state reference");
    expectRejected([](json& j) { j["notes"][0]["written"] = 53; }, "written below range");
    expectRejected([](json& j) { j["notes"][0]["written"] = 90; }, "written above range");
    expectRejected([](json& j) { j["notes"][0]["partial"] = 0; }, "partial 0");
    expectRejected([](json& j) { j["notes"][0]["partial"] = 5; }, "partial above mode count");
    expectRejected([](json& j) { j["notes"][0]["fscale"] = 1.3; }, "fscale out of range");
    expectRejected([](json& j) { j["notes"][0]["natural_dev_cents"] = 150.0; }, "natural deviation out of range");
    expectRejected([](json& j) { j["notes"][0]["kind"] = "other"; }, "bad kind");
    expectRejected([](json& j) { j["notes"].push_back(j["notes"][0]); }, "duplicate note");
    expectRejected([](json& j) { j["notes"] = json::array(); }, "no notes");
    expectRejected([](json& j) { j["radiation"][0]["type"] = "comb"; }, "bad radiation type");
    expectRejected([](json& j) { j["radiation"][0]["f_hz"] = 0.0; }, "radiation frequency 0");
    expectRejected([](json& j) { j["radiation"][0]["q"] = 50.0; }, "radiation q out of range");
    expectRejected([](json& j) { j["radiation"] = json::array(); }, "no radiation sections");
    expectRejected([](json& j) { j["nlp"]["length_m"] = 3.0; }, "nlp length out of range");
    expectRejected([](json& j) { j["nlp"]["beta"] = 0.0; }, "nlp beta out of range");
    expectRejected([](json& j) { j["air"]["c"] = "fast"; }, "wrong type");
    expectRejected([](json& j) { j["air"].erase("rho"); }, "missing air density");
}

TEST_CASE("T-007 embedded table: structure and coverage of the fingering fixture", "[T-007][unit]") {
    const auto t = tpttest::embeddedTable();
    for (size_t i = 0; i < t->stateCount(); ++i) {
        const auto& st = t->state(i);
        INFO("state " << st.id);
        REQUIRE(st.modes.size() >= 13);
        for (size_t k = 0; k < st.modes.size(); ++k) {
            CHECK(st.modes[k].s.real() < 0.0);
            if (k > 0) CHECK(st.modes[k].s.imag() > st.modes[k - 1].s.imag());
        }
    }
    for (const auto& f : tpttest::loadFingeringFixture()) {
        const auto* e = t->note(f.written, f.alternate);
        INFO("written " << f.written << (f.alternate ? " alt" : " std"));
        REQUIRE(e != nullptr);
        const auto& st = t->state(static_cast<size_t>(e->stateIndex));
        CHECK(tpt::valvesName(st.valves) == f.valves);
        CHECK(e->partial == f.partial);
        CHECK(st.trigger == (f.trigger == "-" ? tpt::Trigger::None : f.trigger == "1" ? tpt::Trigger::First : tpt::Trigger::Third));
        CHECK(e->fscale >= 0.8);
        CHECK(e->fscale <= 1.25);
    }
}

TEST_CASE("T-007 open state matches the measured trumpet (Freour 2022 Table 1)", "[T-007][unit]") {
    const auto t = tpttest::embeddedTable();
    const auto meas = tpttest::loadMeasuredModes();
    REQUIRE(meas.size() == 11);
    const auto& open = t->state(t->stateFor(0));
    for (size_t k = 0; k < meas.size(); ++k) {
        const double fm = meas[k].im / (2 * M_PI);
        const double ft = open.modes[k].s.imag() / (2 * M_PI);
        INFO("mode " << k + 1);
        CHECK(std::fabs(1200 * std::log2(ft / fm)) <= (k == 0 ? 25.0 : 10.0));
        const double hm = 20 * std::log10(std::abs(std::complex<double>(meas[k].cre, meas[k].cim)) / -meas[k].re);
        const double ht = 20 * std::log10(std::abs(open.modes[k].R) / -open.modes[k].s.real());
        CHECK(std::fabs(ht - hm) <= 1.5);
    }
}

TEST_CASE("T-007 valve loops and natural tendencies", "[T-007][unit]") {
    const auto t = tpttest::embeddedTable();
    const auto& open = t->state(t->stateFor(0));
    auto meanLowering = [&](tpt::Valves v) {
        const auto& s = t->state(t->stateFor(v));
        double sum = 0;
        for (int k = 3; k <= 6; ++k) sum += 1200 * std::log2(open.modes[size_t(k - 1)].s.imag() / s.modes[size_t(k - 1)].s.imag());
        return sum / 4.0;
    };
    CHECK(std::fabs(meanLowering(tpt::kValve2) - 100.0) <= 5.0);
    CHECK(std::fabs(meanLowering(tpt::kValve1) - 200.0) <= 5.0);
    CHECK(std::fabs(meanLowering(tpt::kValve3) - 300.0) <= 5.0);
    // 1-3 and 1-2-3 on partial 3 are sharp relative to the open partial 3 (C-064, C-079)
    auto relDev = [&](tpt::Valves v, int k) {
        const auto& s = t->state(t->stateFor(v));
        return 1200 * std::log2(s.modes[size_t(k - 1)].s.imag() / open.modes[size_t(k - 1)].s.imag()) + 100.0 * tpt::loweringSemitones(v);
    };
    CHECK(relDev(tpt::kValve1 | tpt::kValve3, 3) >= 15.0);
    CHECK(relDev(tpt::kValve1 | tpt::kValve2 | tpt::kValve3, 3) >= 15.0);
    // naturalDevCents == hs_dev(n) + [dev(state, n) - dev(open, n)] (D-011), recomputed here
    for (const auto& e : t->notes()) {
        const auto& st = t->state(static_cast<size_t>(e.stateIndex));
        const int n = e.partial;
        const double hs = 1200.0 * std::log2(double(n)) - 100.0 * std::round(12.0 * std::log2(double(n)));
        const double fState = st.modes[size_t(n - 1)].s.imag() / (2 * M_PI);
        const double fOpen = open.modes[size_t(n - 1)].s.imag() / (2 * M_PI);
        const double devState = 1200 * std::log2(fState / tpt::equalTemperedHz(e.written - 2));
        const double devOpen = 1200 * std::log2(fOpen / tpt::equalTemperedHz(48 + int(std::lround(12.0 * std::log2(double(n)))) - 2));
        INFO("written " << e.written << (e.alternate ? " alt" : ""));
        CHECK(e.naturalDevCents == Approx(hs + devState - devOpen).margin(0.01));
    }
}
