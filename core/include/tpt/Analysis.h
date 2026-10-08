// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstddef>
#include <vector>

namespace tpt {

/// YIN fundamental estimate (de Cheveigne & Kawahara 2002: difference function, cumulative-mean normalisation,
/// absolute threshold 0.1, parabolic interpolation) over the last min(n, 1.0 s) samples, searching [fminHz, fmaxHz].
/// Returns 0 for silence (RMS < 1e-6) or when no dip below the threshold exists.
double estimateF0(const float* x, std::size_t n, double sampleRate, double fminHz = 50.0, double fmaxHz = 2500.0);

/// Levels (dB, relative to the strongest of them) of harmonics 1..count of f0: Hann window, zero-padding to the next
/// power of two >= 4 n, maximum magnitude bin within +-3 % of h f0. Same definition as tools/realism/metrics.py and
/// research/spikes/ref_spread.py. Throws std::invalid_argument if f0 <= 0, n == 0 or count < 1.
std::vector<double> harmonicLevelsDb(const float* x, std::size_t n, double sampleRate, double f0, int count);

/// Power-spectrum centroid (Hz) over [20 Hz, min(10 kHz, fs/2)], Hann window. 0 for silence.
double spectralCentroidHz(const float* x, std::size_t n, double sampleRate);

/// 1-based index of the resonance nearest to f0 on a log-frequency scale; 0 if f0 <= 0 or the list is empty.
int nearestPartial(double f0, const std::vector<double>& resonanceHz);

/// RMS level in dBFS (20 log10 rms); -200 for silence.
double rmsDb(const float* x, std::size_t n);

} // namespace tpt
