// SPDX-License-Identifier: Apache-2.0
// Prints the real-time factor of the T-024b loop (the frozen test prints it only on failure): "RTF <value>".
#include "TestSupport.h"
#include <chrono>
#include <cstdio>

int main() {
    const double fs = 48000;
    auto v = tpttest::makeVoice(fs, 128);
    std::vector<float> buf(128);
    const int blocks = static_cast<int>(20.0 * fs / 128);
    const auto t0 = std::chrono::steady_clock::now();
    tpt::VoiceParameters p; p.breathNoise = 0.3f; p.vibratoDepth = 0.5f;
    for (int b = 0; b < blocks; ++b) {
        if (b % 300 == 0) { v->noteOn(52 + (b / 300) % 36, 0.9f); p.overblow = (b / 300) % 3 == 0 ? 0.6f : 0.0f; v->setParameters(p); }
        v->process(buf.data(), 128);
    }
    const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("RTF %.5f\n", secs / 20.0);
    return 0;
}
