# GATE-G-006: host acceptance in REAPER

## What was done
- The release-candidate build of commit `404f981` was uploaded after your G-002 approval (`GATE-G-002.md`, Outcome). All CI jobs are green on that commit: freeze, core on three systems,
  python, plugin on three systems (pluginval at strictness 10, auval on macOS, MSVC static-runtime check on Windows, ASan on Linux), build-linux and realism.
- G-005 is closed (`proceed`); the frozen realism tests T-029, T-021 and T-022 pass, so there are no accepted failures.

## Evidence and how to get it
- Run: https://github.com/qualitycoding/trumpet-vst/actions/runs/41086127699 (scroll to **Artifacts** at the bottom of the page; you must be logged in to GitHub).
- **Windows 11:** `trumpet-rc-404f981-windows-2025` (VST3 and Standalone). **macOS:** `trumpet-rc-404f981-macos-15` (VST3, AU, Standalone). The SHA-256 values are in `GATE-G-002.md`.
- Each zip also holds the licence texts and `BUILD_INFO.txt`. The plugin editor shows `Build 404f981` at its bottom right corner: check that it matches.
- **Download and keep the artifact: it expires on 2027-01-28.**

## Install (Windows)
1. Unzip the artifact. Copy the folder `VST3\Trumpet.vst3` to `C:\Program Files\Common Files\VST3\` (administrator rights needed).
2. In REAPER: Options > Preferences > Plug-ins > VST > Re-scan. The instrument is called "Trumpet". The build is unsigned: Windows SmartScreen may warn for `Standalone\Trumpet.exe`.
3. Quick check without REAPER: run `Standalone\Trumpet.exe`, choose a MIDI keyboard and an audio output in its settings.

## Checklist (for you)
1. REAPER scans and loads "Trumpet" without errors.
2. A MIDI item with notes written F#3-C6 plays at the right pitches; mod wheel gives vibrato; pitch bend works.
3. The editor shows valves, slide triggers and partials matching the notes.
4. CC16 (or the Overblow knob) jumps partials; Fixed valves with keyswitches 24-31 works.
5. Save the project, close and reopen REAPER: parameters and held valves are restored.
6. Switch the project sample rate 44.1 <-> 48 kHz with the editor open: no crash, pitch still right.
7. Render to WAV: no clicks or dropouts at 128-sample buffers.
8. Repeated notes and detached phrases articulate (each note re-tongued); overlapping notes slur (R-029).
9. An item that starts with CC11 = 127 still plays soft notes soft, and CC11 = 64 halves the level (R-030).

(Notes are MIDI concert pitch: the instrument is a B-flat trumpet, so written C4 is MIDI 58. MIDI notes 24-31 are keyswitches in Fixed-valves mode and never sound.)

## Allowed responses (verbatim)
`proceed`, `issues: <numbered list>`, `stop`

## Other open items
- Optional, no reply needed to proceed: if you may build locally later, paste the output of `cmake --version` and the generator list of `cmake --help` (recorded in `MAINTENANCE.md`).
- The Windows `BUILD_INFO.txt` names the MinGW `c++` on the runner's PATH as the compiler; the plugin itself was built with MSVC (`GATE-G-002.md`, Outcome).
- If you answer `issues`: I fix them (a fix that touches the sound re-opens G-005), ask G-002 again for the new commit and re-issue this gate with a new artifact.
