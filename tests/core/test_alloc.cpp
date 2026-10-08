// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-024a no heap allocation on the audio path (A-012, D-006): every real-time member of TrumpetVoice is called with a
// counting global operator new active.
#include "TestSupport.h"
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <cstdlib>
#include <new>

namespace {
std::atomic<bool> g_counting{false};
std::atomic<long> g_allocs{0};
} // namespace

void* operator new(std::size_t n) {
    if (g_counting.load()) g_allocs.fetch_add(1);
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

TEST_CASE("T-024 no allocation in real-time calls", "[T-024][alloc]") {
    auto v = tpttest::makeVoice(48000, 512);
    std::vector<float> buf(512);
    v->process(buf.data(), 512);                 // warm-up outside the counted region
    g_allocs = 0;
    g_counting = true;
    tpt::VoiceParameters p;
    for (int i = 0; i < 400; ++i) {
        if (i % 50 == 0) { v->noteOn(55 + (i / 50) * 3, 0.7f); }
        if (i % 50 == 25) { v->noteOff(55 + (i / 50) * 3); }
        if (i % 10 == 0) { p.overblow = (i % 20 == 0) ? 0.7f : -0.7f; p.fixedValves = (i % 100 == 0); v->setParameters(p); }
        v->setBreath(static_cast<float>(i % 100) / 100.0f);
        v->setPitchBend(0.5f);
        v->setVibratoControl(0.5f);
        v->setOverblowControl(0.1f);
        v->keyswitch(26);
        (void)v->uiState();
        (void)v->latencySamples();
        (void)v->soundingHz();
        v->process(buf.data(), 512);
    }
    v->allNotesOff();
    v->reset();
    g_counting = false;
    CHECK(g_allocs.load() == 0);
}
