// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/Analysis.h"
#include "tpt/Fingering.h"
#include "tpt/Pitch.h"
#include "tpt/ResonatorTable.h"
#include "tpt/TrumpetVoice.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tpttest {

struct FixtureFingering { int written; std::string valves; int partial; std::string trigger; bool alternate; };

/// tests/fixtures/trumpet_fingerings.txt: "written valves pN trigger kind" per line, '#' comments.
inline std::vector<FixtureFingering> loadFingeringFixture() {
    std::ifstream in(std::string(TPT_FIXTURE_DIR) + "/trumpet_fingerings.txt");
    if (!in) throw std::runtime_error("missing fingering fixture");
    std::vector<FixtureFingering> out;
    std::string line;
    while (std::getline(in, line)) {
        if (const auto c = line.find('#'); c != std::string::npos) line = line.substr(0, c);
        std::istringstream ss(line);
        FixtureFingering f; std::string p, kind;
        if (!(ss >> f.written)) continue;
        if (!(ss >> f.valves >> p >> f.trigger >> kind) || p.size() < 2 || p[0] != 'p') throw std::runtime_error("bad fixture line: " + line);
        f.partial = std::stoi(p.substr(1));
        f.alternate = kind == "alt";
        out.push_back(f);
    }
    return out;
}

struct MeasuredMode { double re, im, cre, cim; };

/// tests/fixtures/freour2022_open_modes.txt (Freour et al. 2022 Table 1).
inline std::vector<MeasuredMode> loadMeasuredModes() {
    std::ifstream in(std::string(TPT_FIXTURE_DIR) + "/freour2022_open_modes.txt");
    if (!in) throw std::runtime_error("missing measured-modes fixture");
    std::vector<MeasuredMode> out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        MeasuredMode m;
        if (ss >> m.re >> m.im >> m.cre >> m.cim) out.push_back(m);
    }
    return out;
}

inline std::string readFixture(const std::string& name) {
    std::ifstream in(std::string(TPT_FIXTURE_DIR) + "/" + name);
    if (!in) throw std::runtime_error("missing fixture " + name);
    std::stringstream ss; ss << in.rdbuf(); return ss.str();
}

inline std::shared_ptr<const tpt::ResonatorTable> embeddedTable() {
    return std::make_shared<const tpt::ResonatorTable>(tpt::ResonatorTable::fromJson(tpt::embeddedResonatorJson()));
}

/// Dynamics used by all tests (D-008): velocity for pp / mf / ff.
inline constexpr float kVelPP = 24.0f / 127.0f, kVelMF = 76.0f / 127.0f, kVelFF = 124.0f / 127.0f;

/// Nominal concert frequency of partial m of a valve combination: written 48 + 12 log2(m) - lowering, concert = -2.
inline double nominalPartialHz(tpt::Valves v, int m, double a4 = 440.0) {
    const double written = 48.0 + 12.0 * std::log2(double(m)) - tpt::loweringSemitones(v);
    return a4 * std::pow(2.0, (written - 2.0 - 69.0) / 12.0);
}

/// A voice prepared at fs with block size `block` and parameters p (breath controller absent).
inline std::unique_ptr<tpt::TrumpetVoice> makeVoice(double fs, int block = 256, const tpt::VoiceParameters& p = {}) {
    auto v = std::make_unique<tpt::TrumpetVoice>(embeddedTable());
    v->prepare(fs, block);
    v->setParameters(p);
    return v;
}

/// Renders `seconds` of audio from an already started voice, in blocks of `block` samples.
inline std::vector<float> render(tpt::TrumpetVoice& v, double fs, double seconds, int block = 256) {
    std::vector<float> out(static_cast<size_t>(seconds * fs + 0.5));
    for (size_t i = 0; i < out.size(); i += static_cast<size_t>(block)) {
        const int n = static_cast<int>(std::min<size_t>(static_cast<size_t>(block), out.size() - i));
        v.process(out.data() + i, n);
    }
    return out;
}

/// Renders a single held note of `seconds` (note on at t = 0).
inline std::vector<float> renderNote(double fs, int concert, float velocity, double seconds,
                                     const tpt::VoiceParameters& p = {}, int block = 256) {
    auto v = makeVoice(fs, block, p);
    v->noteOn(concert, velocity);
    return render(*v, fs, seconds, block);
}

/// Last `seconds` of x.
inline std::vector<float> tail(const std::vector<float>& x, double fs, double seconds) {
    const size_t n = std::min(x.size(), static_cast<size_t>(seconds * fs));
    return std::vector<float>(x.end() - static_cast<long>(n), x.end());
}

inline double f0Of(const std::vector<float>& x, double fs) { return tpt::estimateF0(x.data(), x.size(), fs, 40.0, 2500.0); }

/// Cents of f relative to the nearest of the given frequencies, and which (1-based) it is.
inline std::pair<double, int> nearestCents(double f, const std::vector<double>& refs) {
    double best = 1e9; int idx = 0;
    for (size_t i = 0; i < refs.size(); ++i) {
        const double c = 1200.0 * std::log2(f / refs[i]);
        if (std::fabs(c) < std::fabs(best)) { best = c; idx = static_cast<int>(i) + 1; }
    }
    return {best, idx};
}

/// Nominal partial index (1..13) of frequency f on valve combination v (0 if f <= 0).
inline int nominalPartialOf(double f, tpt::Valves v, double a4 = 440.0) {
    if (f <= 0) return 0;
    std::vector<double> refs;
    for (int m = 1; m <= 13; ++m) refs.push_back(nominalPartialHz(v, m, a4));
    return nearestCents(f, refs).second;
}

inline bool allFinite(const std::vector<float>& x) {
    return std::all_of(x.begin(), x.end(), [](float s) { return std::isfinite(s); });
}

} // namespace tpttest
