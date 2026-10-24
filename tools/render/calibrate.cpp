// SPDX-License-Identifier: Apache-2.0
// tpt_calibrate (D-011, D-026 R-023): per-note tuning calibration of data/trumpet_resonators.json, and the read-only
// --long-hold mode.
//   tpt_calibrate [--notes <first>-<last>] <json>              calibrate fscale (96 kHz, mf, YIN on the last 0.5 s)
//   tpt_calibrate --long-hold [--notes <first>-<last>] <json>  render every standard note at mf for 20 s (48 kHz)
// Logs: logs/S-009/calibrate.log and logs/S-009-long-hold.md (appended; relative to the working directory).
#include "VoiceTuning.h"
#include "tpt/Analysis.h"
#include "tpt/Fingering.h"
#include "tpt/Pitch.h"
#include "tpt/ResonatorTable.h"
#include "tpt/TrumpetVoice.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

constexpr float kVelMF = 76.0f / 127.0f;

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void writeFileAtomic(const std::string& path, const std::string& text) {
    const std::string tmp = path + ".tmp";
    { std::ofstream out(tmp, std::ios::binary); out << text; }
    std::filesystem::rename(tmp, path);
}

double round10(double x) { char b[64]; std::snprintf(b, sizeof b, "%.10g", x); return std::atof(b); }

std::unique_ptr<tpt::TrumpetVoice> makeVoice(const json& table, double fs, bool alternates) {
    auto t = std::make_shared<const tpt::ResonatorTable>(tpt::ResonatorTable::fromJson(table.dump()));
    auto v = std::make_unique<tpt::TrumpetVoice>(t);
    v->prepare(fs, 512);
    tpt::VoiceParameters p;
    p.useAlternates = alternates;
    v->setParameters(p);
    return v;
}

std::vector<float> render(tpt::TrumpetVoice& v, double fs, double seconds) {
    std::vector<float> out(static_cast<std::size_t>(seconds * fs + 0.5));
    for (std::size_t i = 0; i < out.size(); i += 512) {
        const int n = static_cast<int>(std::min<std::size_t>(512, out.size() - i));
        v.process(out.data() + i, n);
    }
    return out;
}

/// Cents of the played note relative to 12-TET (NaN if the estimator finds no pitch).
double centsOf(json& table, std::size_t idx, double fscale) {
    table["notes"][idx]["fscale"] = round10(fscale);
    const int written = table["notes"][idx]["written"].get<int>();
    const bool alt = table["notes"][idx]["kind"].get<std::string>() == "alt";
    auto v = makeVoice(table, 96000.0, alt);
    v->noteOn(written - 2, kVelMF);
    const auto x = render(*v, 96000.0, 1.2);
    const std::size_t n = static_cast<std::size_t>(0.5 * 96000.0);
    const double f0 = tpt::estimateF0(x.data() + (x.size() - n), n, 96000.0, 40.0, 2500.0);
    if (f0 <= 0.0) return std::numeric_limits<double>::quiet_NaN();
    return tpt::centsBetween(f0, tpt::equalTemperedHz(written - 2));
}

int nearestHarmonic(double f, tpt::Valves valves) {
    int best = 0;
    double bd = 1e9;
    for (int m = 1; m <= 13; ++m) {
        const double nominal = 440.0 * std::pow(2.0, (48.0 + 12.0 * std::log2(double(m)) - tpt::loweringSemitones(valves) - 2.0 - 69.0) / 12.0);
        const double d = std::fabs(std::log(f / nominal));
        if (d < bd) { bd = d; best = m; }
    }
    return best;
}

tpt::Valves valvesOf(const json& table, const std::string& stateId) {
    for (const auto& st : table["states"])
        if (st["id"].get<std::string>() == stateId) return tpt::parseValves(st["valves"].get<std::string>()).value_or(0);
    return 0;
}

struct Range { int lo = 0, hi = 1000; };

bool inRange(const Range& r, int written) { return written >= r.lo && written <= r.hi; }

std::string stamp(const std::string& what) { return "# " + what + "\n"; }

void appendLog(const std::string& path, const std::string& text) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path().empty() ? "." : std::filesystem::path(path).parent_path());
    std::ofstream out(path, std::ios::app);
    out << text;
}

int calibrate(const std::string& path, const Range& range) {
    json table = json::parse(readFile(path));
    std::ostringstream log;
    log << stamp("tuning " + tpt::tuning::toJson()) << stamp("table " + path + ", notes " + std::to_string(range.lo) + "-" + std::to_string(range.hi));
    log << "written kind fscale cents iterations sustained partial_ok\n";
    int failures = 0;
    for (std::size_t i = 0; i < table["notes"].size(); ++i) {
        const int written = table["notes"][i]["written"].get<int>();
        if (!inRange(range, written)) continue;
        const bool alt = table["notes"][i]["kind"].get<std::string>() == "alt";
        const std::string stateId = table["notes"][i]["state"].get<std::string>();
        double x0 = table["notes"][i]["fscale"].get<double>();
        double c0 = centsOf(table, i, x0);
        int it = 0;
        double x1 = x0, c1 = c0;
        bool ok = std::isfinite(c0);
        if (ok && std::fabs(c0) >= 0.5) {
            x1 = x0 * std::pow(2.0, -c0 / 1200.0);
            for (it = 1; it <= 8; ++it) {
                c1 = centsOf(table, i, x1);
                if (!std::isfinite(c1)) { ok = false; break; }
                if (std::fabs(c1) < 0.5) break;
                // secant in log(fscale)
                const double l0 = std::log(x0), l1 = std::log(x1);
                double slope = (c1 - c0) / (l1 - l0);
                if (!std::isfinite(slope) || std::fabs(slope) < 1e3) slope = 1200.0 / std::log(2.0);
                const double next = std::exp(l1 - c1 / slope);
                x0 = x1; c0 = c1; x1 = next;
            }
            if (it > 8) ok = false;
        } else if (ok) {
            c1 = c0;
        }
        if (ok && (x1 < 0.8 || x1 > 1.25)) ok = false;
        table["notes"][i]["fscale"] = round10(x1);
        // sustain gate and regime lock at 48 kHz (as T-012)
        bool sustained = false, partialOk = false;
        if (ok) {
            auto v = makeVoice(table, 48000.0, alt);
            v->noteOn(written - 2, kVelMF);
            const auto x = render(*v, 48000.0, 2.0);
            const std::size_t a = static_cast<std::size_t>(0.5 * 48000), b = static_cast<std::size_t>(0.3 * 48000);
            const double early = tpt::rmsDb(x.data() + a, b), late = tpt::rmsDb(x.data() + x.size() - b, b);
            sustained = late >= early - 2.0 && late > -60.0;
            const double f0 = tpt::estimateF0(x.data() + (x.size() - b), b, 48000.0, 40.0, 2500.0);
            partialOk = f0 > 0.0 && nearestHarmonic(f0, valvesOf(table, stateId)) == table["notes"][i]["partial"].get<int>();
        }
        const bool good = ok && sustained && partialOk;
        if (!good) ++failures;
        char row[160];
        std::snprintf(row, sizeof row, "%d %s %.10g %.3f %d %d %d%s\n", written, alt ? "alt" : "std", x1, c1, it, sustained, partialOk, good ? "" : "  FAIL");
        log << row;
        std::cout << row << std::flush;
        writeFileAtomic(path, table.dump(1) + "\n");        // partial writes survive an interrupted run
    }
    log << (failures ? "RESULT: " + std::to_string(failures) + " note(s) failed\n" : "RESULT: all notes calibrated\n");
    appendLog("logs/S-009/calibrate.log", log.str());
    return failures ? 1 : 0;
}

/// 20 s at mf, 48 kHz, default parameters: the sounding partial and RMS of every 1 s window (D-026 R-023).
int longHold(const std::string& path, const Range& range) {
    const json table = json::parse(readFile(path));
    const std::string logPath = "logs/S-009-long-hold.md";
    std::ostringstream out;
    if (!std::filesystem::exists(logPath)) {
        out << "# S-009 long hold: every standard note at mf for 20 s (48 kHz)\n\n"
            << "Each cell is `partial/RMS dBFS` of one 1 s window; the partial is `uiState().soundingPartial` at the end of the window.\n"
            << "A note fails if any window's partial differs from the target or RMS(window 20) < RMS(window 1) - 6 dB.\n\n"
            << "| written | target | result | windows 1..20 |\n|---|---|---|---|\n";
    }
    int failures = 0;
    for (std::size_t i = 0; i < table["notes"].size(); ++i) {
        if (table["notes"][i]["kind"].get<std::string>() != "std") continue;
        const int written = table["notes"][i]["written"].get<int>();
        if (!inRange(range, written)) continue;
        const int target = table["notes"][i]["partial"].get<int>();
        auto v = makeVoice(table, 48000.0, false);
        v->noteOn(written - 2, kVelMF);
        std::vector<int> partials;
        std::vector<double> rms;
        for (int w = 0; w < 20; ++w) {
            const auto x = render(*v, 48000.0, 1.0);
            rms.push_back(tpt::rmsDb(x.data(), x.size()));
            partials.push_back(v->uiState().soundingPartial);
        }
        bool fail = rms[19] < rms[0] - 6.0;
        for (int pt : partials) fail = fail || pt != target;
        failures += fail;
        out << "| " << written << " | " << target << " | " << (fail ? "FAIL" : "ok") << " | ";
        for (int w = 0; w < 20; ++w) { char c[32]; std::snprintf(c, sizeof c, "%d/%.0f ", partials[std::size_t(w)], rms[std::size_t(w)]); out << c; }
        out << " |\n";
    }
    appendLog(logPath, out.str());
    std::cout << out.str();
    return failures ? 1 : 0;
}

} // namespace

int main(int argc, char** argv) {
    bool hold = false;
    Range range;
    std::string path;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--long-hold") hold = true;
        else if (a == "--notes" && i + 1 < argc) {
            const std::string r = argv[++i];
            const auto dash = r.find('-');
            if (dash == std::string::npos) { std::cerr << "--notes needs <first>-<last>\n"; return 2; }
            range.lo = std::atoi(r.substr(0, dash).c_str());
            range.hi = std::atoi(r.substr(dash + 1).c_str());
        } else path = a;
    }
    if (path.empty()) { std::cerr << "usage: tpt_calibrate [--long-hold] [--notes <first>-<last>] <table.json>\n"; return 2; }
    try {
        return hold ? longHold(path, range) : calibrate(path, range);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 2;
    }
}
