# trumpet-vst

A physically modelled B♭ trumpet instrument plugin (VST3 / Standalone, plus AU on macOS).

- **Sound:** a lip-valve exciter (the player's lips as a pressure-controlled valve) coupled to a modal
  model of the trumpet bore for each of the eight valve combinations, with nonlinear wave steepening in
  the bore for the brassy *ff* edge — no samples.
- **UI:** draws a trumpet and shows the fingering of the note being played: the three valves (pressed or
  up), the 1st/3rd slide triggers when they are used, and the partial (harmonic) of the valve
  combination that is sounding. Written and concert note names are shown.
- **Overblowing:** an *Overblow* control (more lip tension: brighter, then the note splits and jumps to the
  next partials of the same fingering; below zero it drops toward the pedal tones), lip slurs between
  notes with the same fingering, and a *Fixed valves* mode in which the valves are held and the notes you
  play choose the partial, as on a natural trumpet.

## Status

`main` contains only the licence files and this file. The plugin is implemented on the
**`impl-20261007T145241Z-trumpet-vst-plan`** branch, which has not been merged to `main`. This is the
furthest along of the instrument plugins:

- Built and passing CI on Windows, macOS and Linux: the lip-valve model, the drawn trumpet with valve,
  slide-trigger and partial display, the Overblow control, lip slurs and Fixed valves mode. The frozen
  suites pass, including pluginval at strictness 10, `auval` on macOS and an ASan run on Linux.
- Realism: in the blind listening test (gate G-005), the listener picked the real recording in 7 of 12
  trials in the last round, which is chance level, and the demos were judged good. It was accepted with `proceed`.
- A release candidate (commit `404f981`) was built for Windows and macOS and approved for upload
  (gate G-002). The last open step is a manual host check in REAPER (gate G-006).

The plan is on `gen-20261007T145241Z-trumpet-vst-plan` (start with `HANDOFF.md`).

## Building

Build from the implementation branch, not `main`. You need CMake 3.22 or later, a C++20 compiler and an
internet connection for the pinned JUCE, Catch2 and nlohmann-json downloads:

```bash
git checkout impl-20261007T145241Z-trumpet-vst-plan
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Licence

Source code: [Apache License 2.0](LICENSE). See [NOTICE](NOTICE).
Third-party components (JUCE, VST 3 SDK, etc.) keep their own licences; binaries built with JUCE are
subject to the JUCE licence you build under (AGPLv3 or a commercial JUCE licence).
