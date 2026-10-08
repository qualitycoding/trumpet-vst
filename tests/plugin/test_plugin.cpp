// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-026 plugin processor and editor, headless (Linux: run under xvfb-run -a) (SC-1, SC-2, SC-8; D-010, D-013 .. D-016).
#define CATCH_CONFIG_RUNNER
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "tpt/Valves.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

namespace {
float rmsOf(const juce::AudioBuffer<float>& b, int ch) { return b.getRMSLevel(ch, 0, b.getNumSamples()); }

/// Runs `blocks` blocks of 256 samples; midi events are placed at sample 0 of the first block.
juce::AudioBuffer<float> run(TrumpetAudioProcessor& p, int blocks, juce::MidiBuffer first = {}) {
    juce::AudioBuffer<float> all(2, blocks * 256), buf(2, 256);
    for (int b = 0; b < blocks; ++b) {
        juce::MidiBuffer m;
        if (b == 0) m = first;
        buf.clear();
        p.processBlock(buf, m);
        for (int c = 0; c < 2; ++c) all.copyFrom(c, b * 256, buf, c, 0, 256);
    }
    return all;
}
juce::MidiBuffer msg(std::initializer_list<juce::MidiMessage> ms) {
    juce::MidiBuffer b;
    for (const auto& m : ms) b.addEvent(m, 0);
    return b;
}
} // namespace

TEST_CASE("T-026 parameters (D-016)", "[T-026][plugin]") {
    TrumpetAudioProcessor p;
    struct P { const char* id; float lo, hi, def; };
    const P floats[] = {{"brightness", 0, 1, 0.5f}, {"lipStiffness", 0, 1, 0.5f}, {"breathNoise", 0, 1, 0.1f},
                        {"vibratoRate", 3, 8, 5.5f}, {"vibratoDepth", 0, 1, 0}, {"tuning", 415, 466, 440},
                        {"intonation", 0, 1, 0}, {"gain", -24, 12, 0}, {"overblow", -1, 1, 0}};
    for (const auto& f : floats) {
        auto* prm = dynamic_cast<juce::AudioParameterFloat*>(p.parameters().getParameter(f.id));
        INFO(f.id);
        REQUIRE(prm != nullptr);
        CHECK(prm->range.start == f.lo);
        CHECK(prm->range.end == f.hi);
        CHECK(prm->get() == f.def);
    }
    for (const char* id : {"alternates", "fixedValves"}) {
        auto* prm = dynamic_cast<juce::AudioParameterBool*>(p.parameters().getParameter(id));
        REQUIRE(prm != nullptr);
        CHECK_FALSE(prm->get());
    }
    CHECK(p.getParameters().size() == 11);
}

TEST_CASE("T-026 sound, latency, stereo copy, MIDI routing", "[T-026][plugin]") {
    TrumpetAudioProcessor p;
    p.setPlayConfigDetails(0, 2, 48000.0, 256);
    p.prepareToPlay(48000.0, 256);
    CHECK(p.getLatencySamples() > 0);                     // 2x oversampling decimator at 48 kHz (D-006)
    auto out = run(p, 120, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 90)}));
    CHECK(rmsOf(out, 0) > 0.01f);
    for (int i = 0; i < out.getNumSamples(); ++i) REQUIRE(out.getSample(0, i) == out.getSample(1, i));
    CHECK(p.currentUiState().sounding);
    CHECK(p.currentUiState().concert == 67);
    run(p, 2, msg({juce::MidiMessage::controllerEvent(1, 16, 127)}));       // CC16 Overblow +1
    run(p, 60);
    CHECK(p.currentUiState().targetPartial >= 5);                           // written A4 = 1-2 p4 -> +2 (capped p9)
    run(p, 2, msg({juce::MidiMessage::controllerEvent(1, 16, 64)}));
    run(p, 2, msg({juce::MidiMessage::controllerEvent(1, 123, 0)}));        // all notes off
    out = run(p, 120);
    CHECK(rmsOf(out, 0) < 1e-4f);
    CHECK_FALSE(p.currentUiState().sounding);
    out = run(p, 120, msg({juce::MidiMessage::noteOn(1, 67, (juce::uint8) 90), juce::MidiMessage::pitchWheel(1, 16383)}));
    CHECK(rmsOf(out, 0) > 0.01f);
}

TEST_CASE("T-026 keyswitches in Fixed-valves mode", "[T-026][plugin]") {
    TrumpetAudioProcessor p;
    p.setPlayConfigDetails(0, 2, 48000.0, 256);
    p.prepareToPlay(48000.0, 256);
    p.parameters().getParameter("fixedValves")->setValueNotifyingHost(1.0f);
    auto out = run(p, 40, msg({juce::MidiMessage::noteOn(1, 29, (juce::uint8) 100)}));
    CHECK(rmsOf(out, 0) < 1e-6f);
    CHECK(p.currentUiState().fixedValves);
    CHECK(p.currentUiState().valves == (tpt::kValve1 | tpt::kValve3));
}

TEST_CASE("T-026 state round trip and garbage", "[T-026][plugin]") {
    TrumpetAudioProcessor a, b;
    a.parameters().getParameter("brightness")->setValueNotifyingHost(0.9f);
    a.parameters().getParameter("alternates")->setValueNotifyingHost(1.0f);
    a.parameters().getParameter("tuning")->setValueNotifyingHost(a.parameters().getParameter("tuning")->convertTo0to1(442.0f));
    juce::MemoryBlock mb;
    a.getStateInformation(mb);
    CHECK(mb.getSize() > 0);
    b.setStateInformation(mb.getData(), static_cast<int>(mb.getSize()));
    CHECK(std::fabs(dynamic_cast<juce::AudioParameterFloat*>(b.parameters().getParameter("brightness"))->get() - 0.9f) < 1e-4f);
    CHECK(std::fabs(dynamic_cast<juce::AudioParameterFloat*>(b.parameters().getParameter("tuning"))->get() - 442.0f) < 0.01f);
    CHECK(dynamic_cast<juce::AudioParameterBool*>(b.parameters().getParameter("alternates"))->get());
    const char junk[] = "\x01\x02not a state";
    b.setStateInformation(junk, sizeof(junk));
    CHECK(std::fabs(dynamic_cast<juce::AudioParameterFloat*>(b.parameters().getParameter("brightness"))->get() - 0.9f) < 1e-4f);
}

TEST_CASE("T-026 editor shows the processor state", "[T-026][plugin]") {
    TrumpetAudioProcessor p;
    p.setPlayConfigDetails(0, 2, 48000.0, 256);
    p.prepareToPlay(48000.0, 256);
    std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditor());
    auto* e = dynamic_cast<TrumpetAudioProcessorEditor*>(ed.get());
    REQUIRE(e != nullptr);
    CHECK(e->getWidth() >= 800);
    CHECK(e->getHeight() >= 500);
    run(p, 60, msg({juce::MidiMessage::noteOn(1, 60, (juce::uint8) 90)}));   // concert C4 = written D4 = 1-3 p3, 3rd slide
    e->refreshFromProcessor();
    CHECK(e->view().state() == p.currentUiState());
    CHECK(e->view().state().valves == (tpt::kValve1 | tpt::kValve3));
    CHECK(e->view().state().trigger == tpt::Trigger::Third);
    juce::Image img(juce::Image::ARGB, e->getWidth(), e->getHeight(), true);
    juce::Graphics g(img);
    e->paintEntireComponent(g, false);
    CHECK(img.isValid());
}

int main(int argc, char* argv[]) {
    juce::ScopedJuceInitialiser_GUI juce;
    return Catch::Session().run(argc, argv);
}
