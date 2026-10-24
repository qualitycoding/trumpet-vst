// SPDX-License-Identifier: Apache-2.0
// Line-by-line port of research/spikes/lipsim.cpp (implicit flow, bilinear modes, ramps, attack, yInit, fscale).
#include "tpt/ReferenceSim.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>

namespace tpt {

std::vector<double> simulateReference(const RefSimConfig& cfg) {
    using cd = std::complex<double>;
    constexpr double kPi = 3.14159265358979323846;
    if (cfg.modes.empty()) throw std::invalid_argument("simulateReference: no modes");
    if (!(cfg.sampleRate > 0.0) || !(cfg.durationS > 0.0) || cfg.oversample < 1)
        throw std::invalid_argument("simulateReference: rates and duration must be positive");

    const double fs = cfg.sampleRate * cfg.oversample, T = 1.0 / fs, dur = cfg.durationS;
    const long nOut = std::lround(dur * cfg.sampleRate), N = nOut * cfg.oversample;
    const double fl0 = cfg.lip.flHz, fl1 = cfg.flEndHz < 0 ? fl0 : cfg.flEndHz;
    const double H0 = cfg.lip.h0M, H1 = cfg.h0EndM < 0 ? H0 : cfg.h0EndM;
    const double pm0 = cfg.pmPa, pm1 = cfg.pmEndPa < 0 ? pm0 : cfg.pmEndPa;
    const double rho = cfg.rho, Ql = cfg.lip.Ql, mu = cfg.lip.mu, b = cfg.lip.widthM;

    std::vector<cd> av, gv, q(cfg.modes.size(), cd(0.0, 0.0));
    double Z0 = 0.0;
    for (const Mode& m : cfg.modes) {
        const cd sig = m.s * cfg.fscale, R = m.R * cfg.fscale;
        const cd a = (1.0 + sig * T / 2.0) / (1.0 - sig * T / 2.0), g = R * (T / 2.0) / (1.0 - sig * T / 2.0);
        av.push_back(a);
        gv.push_back(g);
        Z0 += 2 * g.real();
    }
    double y = cfg.yInitM >= 0 ? cfg.yInitM : H0, v = 0.0, u = 0.0, p = 0.0;
    std::vector<double> out;
    out.reserve(static_cast<std::size_t>(nOut));
    const double t0r = cfg.rampStartS, rdur = cfg.rampDurS;
    for (long k = 0; k < N; ++k) {
        const double t = static_cast<double>(k) * T;
        const double frac = t < t0r ? 0.0 : std::min(1.0, (t - t0r) / (rdur > 0 ? rdur : dur - t0r));
        const double env = t < cfg.attackS ? 0.5 * (1 - std::cos(kPi * t / cfg.attackS)) : 1.0;
        const double pm = env * (pm0 + (pm1 - pm0) * frac);
        const double fl = fl0 * std::pow(fl1 / fl0, frac), wl = 2 * kPi * fl;
        const double H = H0 + (H1 - H0) * frac;
        // lip: semi-implicit Euler with the previous pressure
        const double acc = (pm - p) / mu - (wl / Ql) * v - wl * wl * (y - H);
        v += T * acc;
        y += T * v;
        const double h = std::max(y, 0.0);
        // implicit flow: p = P0 + Z0 u, u = c sign(D) sqrt(|pm - p|)
        double P0 = 0.0;
        for (std::size_t n = 0; n < q.size(); ++n) P0 += 2 * (av[n] * q[n] + gv[n] * u).real();
        const double c = b * h * std::sqrt(2.0 / rho), D = pm - P0;
        double unew;
        if (c <= 0) unew = 0;
        else if (D >= 0) { const double s = 0.5 * (-Z0 * c + std::sqrt(Z0 * Z0 * c * c + 4 * D)); unew = c * s; }
        else { const double s = 0.5 * (-Z0 * c + std::sqrt(Z0 * Z0 * c * c - 4 * D)); unew = -c * s; }
        for (std::size_t n = 0; n < q.size(); ++n) q[n] = av[n] * q[n] + gv[n] * (unew + u);
        u = unew;
        p = P0 + Z0 * u;
        if (k % cfg.oversample == 0) out.push_back(p);
    }
    return out;
}

} // namespace tpt
