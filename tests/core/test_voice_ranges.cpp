// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-034 parameter extremes keep the regime, the sustain and the tuning (pre-mortem R-013; lesson
// L-20261007T150600Z: control mappings exercised over their full range). DR-RANGE applies on failure.
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace tpttest;

namespace {
struct Out { double f0; bool sustained; };
Out play(int concert, const tpt::VoiceParameters& p) {
    const double fs = 48000;
    auto v = makeVoice(fs, 256, p);
    v->noteOn(concert, kVelMF);
    const auto x = render(*v, fs, 2.0);
    const size_t a = static_cast<size_t>(0.5 * fs), b = static_cast<size_t>(0.3 * fs);
    const double early = tpt::rmsDb(x.data() + a, b), late = tpt::rmsDb(x.data() + x.size() - b, b);
    // over a whole number of vibrato cycles is not needed: the 1.0 s YIN window averages the modulation
    return {f0Of(tail(x, fs, 1.0), fs), late >= early - 2.0 && late > -60.0};
}
} // namespace

TEST_CASE("T-034 parameter extremes (normal range, mf)", "[T-034][integration]") {
    struct Variant { const char* name; tpt::VoiceParameters p; double tolCents; };
    std::vector<Variant> variants;
    { tpt::VoiceParameters p; p.brightness = 0.0f; variants.push_back({"brightness 0", p, 10.0}); }
    { tpt::VoiceParameters p; p.brightness = 1.0f; variants.push_back({"brightness 1", p, 10.0}); }
    { tpt::VoiceParameters p; p.breathNoise = 1.0f; variants.push_back({"breath noise 1", p, 10.0}); }
    { tpt::VoiceParameters p; p.tuningA4Hz = 415.0f; variants.push_back({"A4 415", p, 10.0}); }
    { tpt::VoiceParameters p; p.tuningA4Hz = 466.0f; variants.push_back({"A4 466", p, 10.0}); }
    { tpt::VoiceParameters p; p.vibratoDepth = 1.0f; p.vibratoRateHz = 3.0f; variants.push_back({"vibrato 3 Hz", p, 15.0}); }
    { tpt::VoiceParameters p; p.vibratoDepth = 1.0f; p.vibratoRateHz = 8.0f; variants.push_back({"vibrato 8 Hz", p, 15.0}); }
    for (const auto& var : variants) {
        for (const auto& f : tpt::fingeringTable()) {
            if (f.alternate || f.written > tpt::kHighestNormalWritten) continue;
            const int concert = f.written - 2;
            const auto r = play(concert, var.p);
            const double ref = tpt::equalTemperedHz(concert, var.p.tuningA4Hz);
            INFO(var.name << " written " << f.written << " f0 " << r.f0);
            CHECK(r.sustained);
            CHECK(nominalPartialOf(r.f0, f.valves, var.p.tuningA4Hz) == f.partial);
            CHECK(std::fabs(1200.0 * std::log2(r.f0 / ref)) <= var.tolCents);
        }
    }
}
