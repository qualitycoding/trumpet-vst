// SPDX-License-Identifier: Apache-2.0
#include "tpt/LipModel.h"
#include <algorithm>
#include <stdexcept>

namespace tpt {

namespace {
constexpr int kMaxPartial = 13;

// f_l / f_res ratio, Doc et al. 2023 Table II frequencies over Freour et al. 2022 Table 1 resonances (D-005).
double ratioFor(int n) {
    static const double r[] = {235.0 / 232.70, 340.0 / 348.07, 467.0 / 462.60, 586.0 / 582.14, 703.0 / 690.58};
    if (n == 1) return 1.01;
    return r[std::min(n, 6) - 2];
}

// Lip opening at rest in metres (Doc et al. 2023 Table II, extrapolated outside registers 2..6).
double h0For(int n) {
    static const double h[] = {0.242, 0.218, 0.190, 0.172, 0.168};
    if (n == 1) return 0.27e-3;
    if (n <= 6) return h[n - 2] * 1e-3;
    return std::max(0.12, 0.168 - 0.006 * (n - 6)) * 1e-3;
}
} // namespace

LipParams lipSettingFor(int partial, double fresHz) {
    if (partial < 1 || partial > kMaxPartial) throw std::invalid_argument("lipSettingFor: partial out of [1, 13]");
    if (!(fresHz > 0.0)) throw std::invalid_argument("lipSettingFor: resonance must be positive");
    LipParams p;
    p.flHz = ratioFor(partial) * fresHz;
    p.Ql = 20.0;
    p.mu = 9.0;
    p.widthM = 12e-3;
    p.h0M = h0For(partial);
    return p;
}

double thresholdPressurePa(int partial) {
    if (partial < 1 || partial > kMaxPartial) throw std::invalid_argument("thresholdPressurePa: partial out of [1, 13]");
    static const double t[] = {1047.0, 1788.0, 2477.0, 3158.0, 4109.0};
    if (partial == 1) return 800.0;
    if (partial <= 6) return t[partial - 2];
    return 4109.0 + 700.0 * (partial - 6);
}

} // namespace tpt
