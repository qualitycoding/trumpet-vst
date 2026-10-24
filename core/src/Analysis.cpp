// SPDX-License-Identifier: Apache-2.0
#include "tpt/Analysis.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>

namespace tpt {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kYinThreshold = 0.1;   // absolute threshold of de Cheveigne & Kawahara 2002

/// In-place radix-2 FFT; a.size() must be a power of two.
void fft(std::vector<std::complex<double>>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * kPi / static_cast<double>(len);
        const std::complex<double> wl(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const auto u = a[i + k], v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

/// Magnitude spectrum (bins 0..N/2) of the Hann-windowed signal, zero-padded to the next power of two >= 4 n.
/// Window: symmetric Hann, as numpy.hanning (tools/realism/metrics.py).
std::vector<double> hannMagnitude(const float* x, std::size_t n, std::size_t& nfft) {
    nfft = 1;
    while (nfft < 4 * n) nfft <<= 1;
    std::vector<std::complex<double>> a(nfft, {0.0, 0.0});
    const double denom = n > 1 ? static_cast<double>(n - 1) : 1.0;
    for (std::size_t i = 0; i < n; ++i)
        a[i] = static_cast<double>(x[i]) * (0.5 - 0.5 * std::cos(2.0 * kPi * static_cast<double>(i) / denom));
    fft(a);
    std::vector<double> mag(nfft / 2 + 1);
    for (std::size_t k = 0; k < mag.size(); ++k) mag[k] = std::abs(a[k]);
    return mag;
}

bool isSilent(const float* x, std::size_t n) {
    double e = 0.0;
    for (std::size_t i = 0; i < n; ++i) e += static_cast<double>(x[i]) * x[i];
    return n == 0 || e == 0.0;
}

} // namespace

double estimateF0(const float* x, std::size_t n, double sampleRate, double fminHz, double fmaxHz) {
    n = std::min(n, static_cast<std::size_t>(sampleRate));   // last 1.0 s
    if (n < 8) return 0.0;
    double e = 0.0;
    for (std::size_t i = 0; i < n; ++i) e += static_cast<double>(x[i]) * x[i];
    if (std::sqrt(e / static_cast<double>(n)) < 1e-6) return 0.0;

    const std::size_t tauMin = std::max<std::size_t>(2, static_cast<std::size_t>(sampleRate / fmaxHz));
    const std::size_t tauMax = std::min(static_cast<std::size_t>(sampleRate / fminHz), n / 2);
    if (tauMax <= tauMin + 2) return 0.0;
    const std::size_t w = n - tauMax;

    std::vector<double> d(tauMax + 2, 0.0), cm(tauMax + 2, 1.0);
    for (std::size_t tau = 1; tau <= tauMax + 1 && tau + w <= n; ++tau) {
        double s = 0.0;
        for (std::size_t j = 0; j < w; ++j) {
            const double diff = static_cast<double>(x[j]) - static_cast<double>(x[j + tau]);
            s += diff * diff;
        }
        d[tau] = s;
    }
    double running = 0.0;
    for (std::size_t tau = 1; tau <= tauMax + 1; ++tau) {
        running += d[tau];
        cm[tau] = running > 0.0 ? d[tau] * static_cast<double>(tau) / running : 1.0;
    }

    std::size_t best = 0;
    for (std::size_t tau = tauMin; tau <= tauMax; ++tau) {
        if (cm[tau] < kYinThreshold) {
            while (tau + 1 <= tauMax && cm[tau + 1] < cm[tau]) ++tau;
            best = tau;
            break;
        }
    }
    if (best == 0) return 0.0;

    double tau = static_cast<double>(best);
    if (best > 1 && best + 1 <= tauMax + 1) {
        const double a = cm[best - 1], b = cm[best], c = cm[best + 1];
        const double den = a - 2.0 * b + c;
        if (std::fabs(den) > 1e-18) tau += 0.5 * (a - c) / den;
    }
    return sampleRate / tau;
}

std::vector<double> harmonicLevelsDb(const float* x, std::size_t n, double sampleRate, double f0, int count) {
    if (!(f0 > 0.0) || n == 0 || count < 1) throw std::invalid_argument("harmonicLevelsDb: bad argument");
    std::size_t nfft = 0;
    const auto mag = hannMagnitude(x, n, nfft);
    const double df = sampleRate / static_cast<double>(nfft);
    std::vector<double> lv(static_cast<std::size_t>(count));
    for (int h = 1; h <= count; ++h) {
        // bins whose frequency lies in [0.97 h f0, 1.03 h f0): numpy searchsorted(left) on both edges
        const auto lo = static_cast<std::size_t>(std::ceil(h * f0 * 0.97 / df));
        const auto hi = std::min(static_cast<std::size_t>(std::ceil(h * f0 * 1.03 / df)), mag.size());
        double m = 1e-12;
        for (std::size_t k = lo; k < hi; ++k) m = std::max(m, mag[k]);
        lv[static_cast<std::size_t>(h - 1)] = 20.0 * std::log10(m + 1e-12);
    }
    const double top = *std::max_element(lv.begin(), lv.end());
    for (auto& v : lv) v -= top;
    return lv;
}

double spectralCentroidHz(const float* x, std::size_t n, double sampleRate) {
    if (isSilent(x, n)) return 0.0;
    std::size_t nfft = 0;
    const auto mag = hannMagnitude(x, n, nfft);
    const double df = sampleRate / static_cast<double>(nfft), top = std::min(10000.0, sampleRate / 2.0);
    double num = 0.0, den = 0.0;
    for (std::size_t k = 0; k < mag.size(); ++k) {
        const double f = static_cast<double>(k) * df;
        if (f < 20.0 || f > top) continue;
        const double p = mag[k] * mag[k];
        num += f * p;
        den += p;
    }
    return den > 0.0 ? num / den : 0.0;
}

int nearestPartial(double f0, const std::vector<double>& resonanceHz) {
    if (!(f0 > 0.0) || resonanceHz.empty()) return 0;
    int best = 0;
    double bestD = 1e300;
    for (std::size_t i = 0; i < resonanceHz.size(); ++i) {
        if (!(resonanceHz[i] > 0.0)) continue;
        const double d = std::fabs(std::log(f0 / resonanceHz[i]));
        if (d < bestD) { bestD = d; best = static_cast<int>(i) + 1; }
    }
    return best;
}

double rmsDb(const float* x, std::size_t n) {
    if (isSilent(x, n)) return -200.0;
    double e = 0.0;
    for (std::size_t i = 0; i < n; ++i) e += static_cast<double>(x[i]) * x[i];
    return std::max(-200.0, 10.0 * std::log10(e / static_cast<double>(n)));
}

} // namespace tpt
