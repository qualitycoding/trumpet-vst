// SPDX-License-Identifier: Apache-2.0
// Every non-frozen constant of the voice (D-005, D-007, D-008, D-009, D-012, D-023), each citing its decision. These
// are the constants S-016 may adjust (DR-REAL, DR-REGIME, DR-ARTIC).
#pragma once
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace tpt::tuning {

// --- D-007 attack and release ("tongue release") ---------------------------------------------------------------------
inline constexpr double kAttackRiseS = 0.015;      // t_r: raised-cosine pressure rise
inline constexpr double kAttackRelaxS = 0.150;     // relaxation from pm_on to pm_sus
inline constexpr double kAccLow = 1.0;             // k_acc for L < 0.35
inline constexpr double kAccHigh = 1.5;            // k_acc for L >= 0.35
inline constexpr double kAccBoundaryL = 0.35;
inline constexpr double kY0Low = 0.0;              // y0_frac for L <= 0.75
inline constexpr double kY0High = 0.5;             // y0_frac for L > 0.75
inline constexpr double kY0BoundaryL = 0.75;
inline constexpr double kBoundaryBlend = 0.05;     // linear interpolation across +-0.05 around each boundary
inline constexpr double kOnsetMinThresholdMul = 1.3;   // pm_on >= 1.3 x threshold
inline constexpr double kReleaseS = 0.040;         // pm falls to 0 on the last note-off
// --- D-010 re-articulation -------------------------------------------------------------------------------------------
inline constexpr double kTongueStopS = 0.025;      // lips closed, pm = 0
inline constexpr double kReattackS = 0.003;        // raised-cosine re-attack, no accent
inline constexpr double kLegatoPedalWindowS = 0.020;   // D-014 CC68
inline constexpr double kRegisterRampS = 0.020;    // D-009/D-010 f_l, h0, pm, fscale ramps
inline constexpr double kValveRampS = 0.015;       // D-010 mode interpolation
// --- D-008 dynamics map ------------------------------------------------------------------------------------------------
inline constexpr double kSusLow = 0.8;             // k_sus for L <= 0.2
inline constexpr double kSusMid = 2.5;             // at L = 0.6
inline constexpr double kSusHigh = 5.0;            // at L = 1.0
inline constexpr double kSusL0 = 0.2, kSusL1 = 0.6, kSusL2 = 1.0;
inline constexpr double kPmSmoothS = 0.010;        // 10 ms one-pole at control rate
// --- D-009 overblow ---------------------------------------------------------------------------------------------------
inline constexpr double kBrightenUp = 0.9;         // pm x (1 + 0.9 r), r in [0, 1)
inline constexpr double kBrightenDown = 0.15;      // pm x (1 + 0.15 r), r in (-1, 0]
inline constexpr int kCapPartial = 9;              // target partial <= max(n, 9)
inline constexpr double kDownHysteresis = 0.2;
// --- D-023 blowing floors ---------------------------------------------------------------------------------------------
inline constexpr double kFloorExtended = 2.5;      // written 86, 87, 89
inline constexpr double kFloorWritten88 = 5.0;
// --- D-005 lip opening scale: h0 x (1.15 - 0.3 lipStiffness) ---------------------------------------------------------
inline constexpr double kStiffBase = 1.15, kStiffSlope = 0.3;
// per-partial trims (DR-REGIME), index = partial 1..13 (slot 0 unused); all 1.0 initially
// partial 3: fl x 0.98 so that written C#4 (123 + 3rd slide) grows fast enough at mf (DR-REGIME round 1, S-009)
inline constexpr std::array<double, 14> kFlTrim = {1, 1, 1, 0.98, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
inline constexpr std::array<double, 14> kH0Trim = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
inline constexpr std::array<double, 14> kPmTrim = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
// --- D-011 vibrato ----------------------------------------------------------------------------------------------------
inline constexpr double kVibratoCents = 25.0;
inline constexpr double kParamSmoothS = 0.010;     // smoothing of bend, realism, brightness, gain
// --- D-012 output stage -----------------------------------------------------------------------------------------------
inline constexpr double kCupRadiusM = 8.25e-3;
inline constexpr double kWarpClampPa = 30000.0;
inline constexpr double kNoiseCutoffHz = 3000.0;
inline constexpr double kNoiseFlowGain = 0.05;
inline constexpr double kNoiseOutGain = 0.002;
inline constexpr unsigned kNoiseSeed = 0x7A11C0DEu;
inline constexpr double kOutGain = 1.0 / 45000.0;  // K_out: written C5 at ff peaks near -6 dBFS (calibrated in S-016)
inline constexpr double kLimiterKnee = 0.9;
// --- D-006 / D-013 ------------------------------------------------------------------------------------------------------
inline constexpr double kFlowRhoKgM3 = 1.2041;     // lipsim value
inline constexpr double kSafetyPressurePa = 1e6;
inline constexpr double kIdleDb = -100.0;
inline constexpr double kIdleHoldS = 0.050;
inline constexpr double kMeterWindowS = 0.020;
inline constexpr double kSoundingDb = -60.0;
inline constexpr double kPanicFadeS = 0.004;       // D-014 allNotesOff
inline constexpr double kTrackerHysteresis = 0.05;

/// All constants as (name, value) pairs, in a fixed order (printed by tpt_render --print-tuning, S-016).
inline std::vector<std::pair<std::string, double>> table() {
    std::vector<std::pair<std::string, double>> t = {
        {"kAccBoundaryL", kAccBoundaryL}, {"kAccHigh", kAccHigh}, {"kAccLow", kAccLow},
        {"kAttackRelaxS", kAttackRelaxS}, {"kAttackRiseS", kAttackRiseS}, {"kBoundaryBlend", kBoundaryBlend},
        {"kBrightenDown", kBrightenDown}, {"kBrightenUp", kBrightenUp}, {"kCapPartial", double(kCapPartial)},
        {"kCupRadiusM", kCupRadiusM}, {"kDownHysteresis", kDownHysteresis}, {"kFloorExtended", kFloorExtended},
        {"kFloorWritten88", kFloorWritten88}, {"kFlowRhoKgM3", kFlowRhoKgM3}, {"kIdleDb", kIdleDb},
        {"kIdleHoldS", kIdleHoldS}, {"kLegatoPedalWindowS", kLegatoPedalWindowS}, {"kLimiterKnee", kLimiterKnee},
        {"kMeterWindowS", kMeterWindowS}, {"kNoiseCutoffHz", kNoiseCutoffHz}, {"kNoiseFlowGain", kNoiseFlowGain},
        {"kNoiseOutGain", kNoiseOutGain}, {"kOnsetMinThresholdMul", kOnsetMinThresholdMul}, {"kOutGain", kOutGain},
        {"kPanicFadeS", kPanicFadeS}, {"kParamSmoothS", kParamSmoothS}, {"kPmSmoothS", kPmSmoothS},
        {"kReattackS", kReattackS}, {"kRegisterRampS", kRegisterRampS}, {"kReleaseS", kReleaseS},
        {"kSafetyPressurePa", kSafetyPressurePa}, {"kSoundingDb", kSoundingDb}, {"kStiffBase", kStiffBase},
        {"kStiffSlope", kStiffSlope}, {"kSusHigh", kSusHigh}, {"kSusL0", kSusL0}, {"kSusL1", kSusL1}, {"kSusL2", kSusL2},
        {"kSusLow", kSusLow}, {"kSusMid", kSusMid}, {"kTongueStopS", kTongueStopS}, {"kTrackerHysteresis", kTrackerHysteresis},
        {"kValveRampS", kValveRampS}, {"kVibratoCents", kVibratoCents}, {"kWarpClampPa", kWarpClampPa},
        {"kY0BoundaryL", kY0BoundaryL}, {"kY0High", kY0High}, {"kY0Low", kY0Low}};
    for (int n = 1; n <= 13; ++n) {
        t.push_back({"kFlTrim" + std::to_string(n), kFlTrim[std::size_t(n)]});
        t.push_back({"kH0Trim" + std::to_string(n), kH0Trim[std::size_t(n)]});
        t.push_back({"kPmTrim" + std::to_string(n), kPmTrim[std::size_t(n)]});
    }
    return t;
}

/// Canonical JSON: one object, keys sorted, numbers as %.10g, no whitespace.
inline std::string toJson() {
    auto t = table();
    std::sort(t.begin(), t.end());
    std::string s = "{";
    for (std::size_t i = 0; i < t.size(); ++i) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "%.10g", t[i].second);
        s += (i ? ",\"" : "\"") + t[i].first + "\":" + buf;
    }
    return s + "}";
}

} // namespace tpt::tuning
