// SPDX-License-Identifier: Apache-2.0
// Breath-noise power (D-012, S-008 action 5): the filtered noise of BreathNoise has the band-power ratios and variance
// of the discrete one-pole H(z) = a / (1 - (1 - a) z^-1) driven by white noise of variance 1/3.
#include "BreathNoise.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <complex>
#include <vector>

namespace {
constexpr double kPi = 3.14159265358979323846;

void fft(std::vector<std::complex<double>>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const std::complex<double> wl(std::cos(-2 * kPi / double(len)), std::sin(-2 * kPi / double(len)));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1, 0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const auto u = a[i + k], v = a[i + k + len / 2] * w;
                a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl;
            }
        }
    }
}

double db(double x) { return 10.0 * std::log10(x); }
} // namespace

TEST_CASE("noise: band powers and variance match the discrete one-pole", "[noise]") {
    for (double fInt : {88200.0, 96000.0, 192000.0}) {
        tpt::BreathNoise noise(fInt);
        const std::size_t n = static_cast<std::size_t>(10.0 * fInt);
        std::vector<double> y(n);
        double mean = 0, var = 0;
        for (auto& s : y) { s = noise.next(); mean += s; }
        mean /= double(n);
        for (double s : y) var += (s - mean) * (s - mean);
        var /= double(n);

        const double a = noise.coefficient();
        auto H2 = [&](double f) {
            const std::complex<double> z = std::polar(1.0, 2 * kPi * f / fInt);
            return std::norm(a / (1.0 - (1.0 - a) / z));
        };
        // Welch periodogram, Hann window, 50 % overlap
        const std::size_t seg = 8192;
        std::vector<double> psd(seg / 2 + 1, 0.0);
        double winPow = 0;
        std::vector<double> win(seg);
        for (std::size_t i = 0; i < seg; ++i) { win[i] = 0.5 - 0.5 * std::cos(2 * kPi * double(i) / double(seg)); winPow += win[i] * win[i]; }
        int count = 0;
        for (std::size_t s0 = 0; s0 + seg <= n; s0 += seg / 2) {
            std::vector<std::complex<double>> buf(seg);
            for (std::size_t i = 0; i < seg; ++i) buf[i] = (y[s0 + i] - mean) * win[i];
            fft(buf);
            for (std::size_t k = 0; k < psd.size(); ++k) psd[k] += std::norm(buf[k]) / winPow;
            ++count;
        }
        for (auto& v : psd) v /= count;                       // variance per bin, summing bins gives the variance (one-sided scaling ignored)
        auto measured = [&](double f1, double f2) {
            double sum = 0;
            for (std::size_t k = 0; k < psd.size(); ++k) {
                const double f = double(k) * fInt / double(seg);
                if (f >= f1 && f < f2) sum += psd[k];
            }
            return sum;
        };
        auto expected = [&](double f1, double f2) {         // (1/3) sum over the same bins of |H|^2 / (number of bins in the full band)
            double sum = 0;
            for (std::size_t k = 0; k < psd.size(); ++k) {
                const double f = double(k) * fInt / double(seg);
                if (f >= f1 && f < f2) sum += H2(f) / 3.0;
            }
            return sum;
        };
        const double bands[4] = {300.0, 1000.0, 3000.0, 8000.0};
        double m[3], e[3];
        for (int b = 0; b < 3; ++b) { m[b] = measured(bands[b], bands[b + 1]); e[b] = expected(bands[b], bands[b + 1]); }
        INFO("f_int " << fInt);
        CHECK(std::fabs((db(m[0]) - db(m[1])) - (db(e[0]) - db(e[1]))) < 0.5);
        CHECK(std::fabs((db(m[1]) - db(m[2])) - (db(e[1]) - db(e[2]))) < 0.5);
        // total variance of the filtered noise: (1/3) a / (2 - a)
        CHECK(std::fabs(db(var) - db(a / (2.0 - a) / 3.0)) < 0.5);
    }
}
