// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/LipModel.h"
#include "tpt/ResonatorTable.h"
#include <vector>

namespace tpt {

/// Offline double-precision reference integrator: a line-by-line port of research/spikes/lipsim.cpp (D-006). Used
/// by tests (T-009) and tools, never by the real-time voice. Parameters ramp linearly from their start to their
/// `...End` value between rampStartS and rampStartS + rampDurS (fl geometrically), as in lipsim.
struct RefSimConfig {
    std::vector<Mode> modes;
    LipParams lip;
    double flEndHz = -1.0;       ///< < 0: equal to lip.flHz
    double h0EndM = -1.0;        ///< < 0: equal to lip.h0M
    double pmPa = 5000.0;
    double pmEndPa = -1.0;       ///< < 0: equal to pmPa
    double rampStartS = 0.0;
    double rampDurS = -1.0;      ///< < 0: until the end
    double attackS = 0.02;       ///< raised-cosine pressure onset
    double yInitM = -1.0;        ///< initial lip opening; < 0: lip.h0M ("tongue release" = 0)
    double sampleRate = 48000.0; ///< output rate
    int oversample = 2;          ///< internal rate = sampleRate * oversample
    double durationS = 1.0;
    double fscale = 1.0;         ///< multiplies every s and R (tuning)
    double rho = 1.2041;         ///< air density, kg/m^3 (lipsim default, 20 degC)
};

/// Mouthpiece pressure (Pa) at sampleRate (every oversample-th internal sample, no filtering), length
/// round(durationS * sampleRate). Throws std::invalid_argument on non-positive rates/durations or empty modes.
std::vector<double> simulateReference(const RefSimConfig& cfg);

} // namespace tpt
