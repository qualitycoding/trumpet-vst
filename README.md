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

Planning. The execution plan lives on the `gen-*-trumpet-vst-plan` branch (start with `HANDOFF.md`).
Nothing is implemented on `main` yet.

## Building

Not yet buildable from `main`. Once implemented: CMake ≥ 3.22, a C++20 compiler, and an internet
connection for the pinned JUCE / Catch2 / nlohmann-json downloads:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Licence

Source code: [Apache License 2.0](LICENSE). See [NOTICE](NOTICE).
Third-party components (JUCE, VST 3 SDK, etc.) keep their own licences; binaries built with JUCE are
subject to the JUCE licence you build under (AGPLv3 or a commercial JUCE licence).
