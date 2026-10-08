// SPDX-License-Identifier: Apache-2.0
#pragma once

namespace tpt {

/// Outward-striking one-mass lip valve (Velut et al. 2017 eq. 1; Freour et al. 2022 eq. 1):
///   h'' + (wl/Ql) h' + wl^2 (h - h0) = (pm - p) / mu,   u = width * max(h, 0) * sign(pm - p) sqrt(2 |pm - p| / rho)
struct LipParams {
    double flHz = 0.0;      ///< lip resonance frequency
    double Ql = 20.0;       ///< lip quality factor
    double mu = 9.0;        ///< surface mass, kg/m^2
    double widthM = 12e-3;  ///< lip channel width
    double h0M = 0.19e-3;   ///< opening at rest
};

/// Default lip setting for playing partial `partial` (1..13) on a resonance at fresHz (D-005, Doc et al. 2023
/// Table II): Ql 20, mu 9, width 12 mm; fl = ratio(partial) * fresHz; h0 per partial. Exact constants in D-005.
/// partial outside [1, 13] or fresHz <= 0 throws std::invalid_argument.
LipParams lipSettingFor(int partial, double fresHz);

/// Oscillation threshold blowing pressure (Pa) used for the dynamics map (D-005, D-008; Doc et al. 2023 Table I
/// medians for partials 2..6, extrapolated per D-005). partial outside [1, 13] throws std::invalid_argument.
double thresholdPressurePa(int partial);

} // namespace tpt
