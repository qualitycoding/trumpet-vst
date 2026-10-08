// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-015 Overblow / Underblow (A-007a, D-009; C-083, C-084). T-016 lip slurs and valve slurs (A-007b, D-010).
// T-017 Fixed-valves mode and keyswitches (A-007c, D-003).
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>

using namespace tpttest;

namespace {
int partialNow(tpt::TrumpetVoice& v, double fs, tpt::Valves valves, double sec) {
    const auto x = render(v, fs, sec);
    return nominalPartialOf(f0Of(tail(x, fs, 0.1), fs), valves);
}
double centroidTail(const std::vector<float>& x, double fs, double sec) {
    const auto t = tail(x, fs, sec);
    return tpt::spectralCentroidHz(t.data(), t.size(), fs);
}
double cents(double f, double ref) { return 1200.0 * std::log2(f / ref); }
} // namespace

TEST_CASE("T-015 register jumps with Overblow and Underblow (normal range, mf)", "[T-015][integration]") {
    const double fs = 48000;
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
        const int n = f.partial;
        const int cap = std::max(n, 9);   // D-009: Overblow never aims above partial max(n, 9) (C-083, C-093)
        tpt::VoiceParameters p;
        auto v = makeVoice(fs, 256, p);
        v->noteOn(f.written - 2, kVelMF);
        INFO("written " << f.written << " partial " << n);
        CHECK(partialNow(*v, fs, f.valves, 0.5) == n);
        p.overblow = 0.6f; v->setParameters(p);
        CHECK(partialNow(*v, fs, f.valves, 0.6) == std::min(n + 1, cap));
        CHECK(v->uiState().targetPartial == std::min(n + 1, cap));
        p.overblow = 1.0f; v->setParameters(p);
        CHECK(partialNow(*v, fs, f.valves, 0.3) == std::min(n + 2, cap));
        CHECK(partialNow(*v, fs, f.valves, 1.0) == std::min(n + 2, cap));   // the new register sustains
        p.overblow = 0.0f; v->setParameters(p);
        CHECK(partialNow(*v, fs, f.valves, 0.4) == n);
        p.overblow = -0.6f; v->setParameters(p);
        CHECK(partialNow(*v, fs, f.valves, 0.6) == n - 1);
        p.overblow = -1.0f; v->setParameters(p);
        CHECK(partialNow(*v, fs, f.valves, 0.3) == std::max(1, n - 2));
        CHECK(partialNow(*v, fs, f.valves, 1.0) == std::max(1, n - 2));
    }
}

TEST_CASE("T-015 Overblow below 0.5 stays on the partial and brightens", "[T-015][integration]") {
    const double fs = 48000;
    int notes = 0, brighter = 0;
    for (const auto& f : tpt::fingeringTable()) {
        if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
        double prev = -1; bool mono = true;
        for (float o : {0.0f, 0.15f, 0.3f, 0.45f}) {
            tpt::VoiceParameters p; p.overblow = o;
            const auto x = renderNote(fs, f.written - 2, kVelMF, 0.8, p);
            INFO("written " << f.written << " overblow " << o);
            CHECK(nominalPartialOf(f0Of(tail(x, fs, 0.3), fs), f.valves) == f.partial);
            const double c = centroidTail(x, fs, 0.4);
            if (prev >= 0 && !(c > prev)) mono = false;
            prev = c;
        }
        ++notes; brighter += mono;
    }
    CHECK(brighter >= static_cast<int>(std::ceil(0.9 * notes)));
}

TEST_CASE("T-015 Overblow via MIDI CC 16 mapping", "[T-015][integration]") {
    const double fs = 48000;
    auto v = makeVoice(fs);
    v->noteOn(58, kVelMF);   // written C4, open p2
    CHECK(partialNow(*v, fs, 0, 0.5) == 2);
    v->setOverblowControl(1.0f);
    CHECK(partialNow(*v, fs, 0, 0.3) == 4);
    v->setOverblowControl(0.0f);
    CHECK(partialNow(*v, fs, 0, 0.4) == 2);
}

TEST_CASE("T-016 lip slur jumps between partials without retriggering", "[T-016][integration]") {
    const double fs = 48000;
    const int seq[] = {58, 65, 70, 74, 77};   // written C4 G4 C5 E5 G5: all open, partials 2..6
    auto v = makeVoice(fs);
    v->noteOn(seq[0], kVelMF);
    auto x = render(*v, fs, 0.5);
    const double steady = tpt::rmsDb(tail(x, fs, 0.2).data(), static_cast<size_t>(0.2 * fs));
    for (int i = 1; i < 5; ++i) {
        v->noteOn(seq[i], kVelMF);       // legato: new note before releasing the old one
        v->noteOff(seq[i - 1]);
        const auto y = render(*v, fs, 0.4);
        INFO("slur to concert " << seq[i]);
        // no dip deeper than 20 dB below the steady level in any 10 ms window
        for (size_t k = 0; k + 480 <= y.size(); k += 480) CHECK(tpt::rmsDb(y.data() + k, 480) > steady - 20.0);
        // target reached within 200 ms and held
        CHECK(std::fabs(cents(f0Of(tail(y, fs, 0.15), fs), tpt::equalTemperedHz(seq[i]))) <= 15.0);
        // jump, not glide: few 20 ms frames in the first 200 ms lie > 60 cents away from both the old and the new note
        int off = 0, frames = 0;
        for (size_t k = 0; k + 960 <= static_cast<size_t>(0.2 * fs); k += 960) {
            const double fk = tpt::estimateF0(y.data() + k, 960, fs, 40.0, 2500.0);
            ++frames;
            if (fk > 0 && std::fabs(cents(fk, tpt::equalTemperedHz(seq[i]))) > 60 && std::fabs(cents(fk, tpt::equalTemperedHz(seq[i - 1]))) > 60) ++off;
        }
        CHECK(off <= frames * 3 / 10);
    }
}

TEST_CASE("T-016 valve slurs on one partial", "[T-016][integration]") {
    const double fs = 48000;
    const int seq[] = {65, 64, 63, 62};   // written G4 (0) F#4 (2) F4 (1) E4 (12), all partial 3
    auto v = makeVoice(fs);
    v->noteOn(seq[0], kVelMF);
    auto x = render(*v, fs, 0.5);
    const double steady = tpt::rmsDb(tail(x, fs, 0.2).data(), static_cast<size_t>(0.2 * fs));
    for (int i = 1; i < 4; ++i) {
        v->noteOn(seq[i], kVelMF);
        v->noteOff(seq[i - 1]);
        const auto y = render(*v, fs, 0.4);
        INFO("valve slur to concert " << seq[i]);
        for (size_t k = 0; k + 480 <= y.size(); k += 480) CHECK(tpt::rmsDb(y.data() + k, 480) > steady - 20.0);
        // pitch within 15 cents from 150 ms after the change
        const std::vector<float> after(y.begin() + static_cast<long>(0.15 * fs), y.end());
        CHECK(std::fabs(cents(f0Of(after, fs), tpt::equalTemperedHz(seq[i]))) <= 15.0);
        CHECK(allFinite(y));
    }
}

TEST_CASE("T-017 Fixed-valves mode and keyswitches", "[T-017][integration]") {
    const double fs = 48000;
    tpt::VoiceParameters p; p.fixedValves = true;
    auto v = makeVoice(fs, 256, p);
    CHECK(v->keyswitch(29));                       // valves 1-3
    auto silent = render(*v, fs, 0.2);
    CHECK(tpt::rmsDb(silent.data(), silent.size()) < -120.0);   // a keyswitch never sounds
    const tpt::Valves v13 = tpt::kValve1 | tpt::kValve3;
    CHECK(v->uiState().valves == v13);             // idle in Fixed-valves mode shows the held valves
    CHECK(v->uiState().fixedValves);
    for (int concert : {53, 60, 65, 68, 72}) {     // partials 2, 3, 4, 5, 6 of 1-3 (nearest nominal); pedal: R-008
        const auto r = tpt::resolveNote(concert, false, v13);
        REQUIRE(r.has_value());
        v->noteOn(concert, kVelMF);
        const auto x = render(*v, fs, 0.6);
        INFO("concert " << concert << " -> partial " << r->fingering.partial);
        CHECK(v->uiState().valves == v13);
        CHECK(v->uiState().targetPartial == r->fingering.partial);
        CHECK(nominalPartialOf(f0Of(tail(x, fs, 0.2), fs), v13) == r->fingering.partial);
        CHECK(std::fabs(cents(f0Of(tail(x, fs, 0.2), fs), nominalPartialHz(v13, r->fingering.partial))) <= 60.0);
        v->noteOff(concert);
        render(*v, fs, 0.4);
    }
    CHECK(v->keyswitch(24));                       // open
    CHECK(v->uiState().valves == 0);
    CHECK_FALSE(v->keyswitch(32));
    // normal mode: keyswitches are consumed but do not change the fingering
    tpt::VoiceParameters q;
    auto w = makeVoice(fs, 256, q);
    CHECK(w->keyswitch(29));
    w->noteOn(58, kVelMF);
    render(*w, fs, 0.5);
    CHECK(w->uiState().valves == 0);
    CHECK_FALSE(w->uiState().fixedValves);
}
