// SPDX-License-Identifier: Apache-2.0
// tpt_f0 <raw float32 mono file> <sample rate>: prints estimateF0 of the last 0.5 s (S-004 cross-check).
#include "tpt/Analysis.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 3) { std::fprintf(stderr, "usage: tpt_f0 <file.raw> <rate>\n"); return 2; }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) { std::fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    std::vector<char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::vector<float> x(bytes.size() / sizeof(float));
    std::copy(bytes.begin(), bytes.begin() + static_cast<long>(x.size() * sizeof(float)), reinterpret_cast<char*>(x.data()));
    const double fs = std::atof(argv[2]);
    const std::size_t n = std::min(x.size(), static_cast<std::size_t>(0.5 * fs));
    std::printf("%.6f\n", tpt::estimateF0(x.data() + (x.size() - n), n, fs));
    return 0;
}
