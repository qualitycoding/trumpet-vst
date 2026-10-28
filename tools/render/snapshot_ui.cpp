// SPDX-License-Identifier: Apache-2.0
// tpt_ui_snapshots [out dir]: 960x600 editor snapshots for gate G-004 (plan/GATES.md). The voice is not run: every image
// is built from a UiState derived from resolveNote and set on the view.
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "tpt/Fingering.h"
#include "tpt/Pitch.h"
#include <cstdio>

namespace {
std::string fileSafeName(int midi) {
    std::string n = tpt::noteName(midi);
    for (auto& c : n)
        if (c == '#') c = 's';
    return n;
}

tpt::UiState stateFor(const tpt::Fingering& f, int targetPartial) {
    tpt::UiState s;
    s.sounding = true;
    s.written = f.written;
    s.concert = f.written - tpt::kTransposition;
    s.valves = f.valves;
    s.trigger = f.trigger;                       // Intonation realism 0: the trigger is shown (D-013)
    s.targetPartial = targetPartial;
    s.soundingPartial = targetPartial;
    s.extended = f.written > tpt::kHighestNormalWritten;
    return s;
}

bool save(TrumpetAudioProcessorEditor& ed, const tpt::UiState& s, const juce::File& file) {
    ed.view().setState(s);
    const juce::Image img = ed.createComponentSnapshot(ed.getLocalBounds(), true, 1.0f);
    if (!img.isValid() || img.getWidth() != 960 || img.getHeight() != 600) return false;
    file.deleteFile();
    juce::FileOutputStream out(file);
    if (!out.openedOk()) return false;
    juce::PNGImageFormat png;
    return png.writeImageToStream(img, out);
}
} // namespace

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File dir = juce::File::getCurrentWorkingDirectory().getChildFile(argc > 1 ? argv[1] : "renders/G-004");
    dir.createDirectory();

    TrumpetAudioProcessor proc;
    proc.setPlayConfigDetails(0, 2, 48000.0, 512);
    proc.prepareToPlay(48000.0, 512);
    std::unique_ptr<juce::AudioProcessorEditor> base(proc.createEditor());
    auto* ed = dynamic_cast<TrumpetAudioProcessorEditor*>(base.get());
    if (!ed) return 2;
    int written = 0, failed = 0;
    auto put = [&](const juce::String& name, const tpt::UiState& s) {
        ++written;
        if (!save(*ed, s, dir.getChildFile(name))) { std::fprintf(stderr, "failed: %s\n", name.toRawUTF8()); ++failed; }
    };

    for (const auto& f : tpt::fingeringTable()) {
        const juce::String name = juce::String(f.alternate ? "alt-written-" : "written-") + juce::String(f.written) + "-" + fileSafeName(f.written) + ".png";
        put(name, stateFor(f, f.partial));
    }
    for (int w : {60, 67, 72, 79, 84}) {
        const auto f = tpt::standardFingering(w);
        if (f) put("overblow-" + juce::String(w) + ".png", stateFor(*f, f->partial + 1));
    }
    tpt::UiState fixedIdle;
    fixedIdle.fixedValves = true;
    fixedIdle.valves = tpt::kValve1 | tpt::kValve3;
    put("fixed-valves.png", fixedIdle);
    if (const auto f = tpt::standardFingering(62)) put("editor.png", stateFor(*f, f->partial));

    std::printf("wrote %d images to %s (%d failed)\n", written, dir.getFullPathName().toRawUTF8(), failed);
    return failed ? 1 : 0;
}
