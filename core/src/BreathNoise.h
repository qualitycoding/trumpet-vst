// SPDX-License-Identifier: Apache-2.0
// Breath noise (D-012): xorshift32 white noise mapped to [-1, 1) through a one-pole low-pass,
// y += a (w - y), a = 1 - exp(-2 pi fc / f_int). Deterministic: the seed is restored by reset().
#pragma once
#include "VoiceTuning.h"
#include <cmath>
#include <cstdint>

namespace tpt {

class BreathNoise {
public:
    explicit BreathNoise(double fInt = 96000.0) { setRate(fInt); reset(); }
    void setRate(double fInt) { a_ = 1.0 - std::exp(-2.0 * 3.14159265358979323846 * tuning::kNoiseCutoffHz / fInt); }
    void reset() { x_ = tuning::kNoiseSeed; y_ = 0.0; }
    /// Next filtered sample (variance of the input is 1/3).
    double next() {
        x_ ^= x_ << 13;
        x_ ^= x_ >> 17;
        x_ ^= x_ << 5;
        const double w = static_cast<double>(x_) / 2147483648.0 - 1.0;
        y_ += a_ * (w - y_);
        return y_;
    }
    double coefficient() const { return a_; }

private:
    double a_ = 0.0, y_ = 0.0;
    std::uint32_t x_ = tuning::kNoiseSeed;
};

} // namespace tpt
