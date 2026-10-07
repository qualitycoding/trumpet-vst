# Assumptions (Phase 0 intake, 2026-10-07)

The intake batch (20 numbered items, each with a proposed default) was sent to the human on 2026-10-07. The
human replied: "Use the defaults for the plan creation". Every item therefore adopts its proposed default as
stated below. Planner-chosen values (not in the intake batch) are marked "planner".

## Intake restatement (0.3.1)
- **Goal:** a plan that a separate implementing agent can carry out without questions, producing a physically
  modelled B♭ trumpet instrument plugin with overblowing and a UI that shows the fingering in use.
- **In scope:** physical-model DSP (lip valve + per-valve-combination bore resonators + nonlinear bore
  propagation), JUCE plugin (VST3/Standalone on Windows/macOS/Linux, AU on macOS), a drawn trumpet UI showing
  valves, slide triggers and partial, overblow/underblow, lip slurs, fixed-valves mode, alternate fingerings,
  expression (velocity, breath CC, pitch bend, vibrato), state persistence, CI on three OSes, automated tests,
  realism comparison against real recordings, human gates.
- **Out of scope:** samples; CLAP/AAX; C trumpet or other brass; half-valve, multiphonics, flutter tongue,
  falls/doits, shakes, mutes; polyphony; network features; telemetry; public release, tags, package publication
  (any of these needs G-002); committing third-party recordings.
- **Success criteria:** SC-1 … SC-10, defined measurably in `plan/TRACEABILITY.md`.
- **Constraints:** C++20; JUCE (pinned in `plan/ENVIRONMENT.md`); CMake ≥ 3.22; Windows x64 (MSVC), macOS
  (AppleClang), Linux x64 (GCC ≥ 12); own source Apache-2.0; JUCE under AGPLv3 or a JUCE licence.
- **Profiles:** `software` only, `software.deploys = false` (`plan/PROFILE.md`).
- **Subject and tags:** a physically modelled brass (trumpet) instrument plugin. Tags: `domain:audio`,
  `domain:musical-acoustics`, `domain:brass`, `tech:cpp`, `tech:juce`, `tech:cmake`, `tech:vst3`,
  `tech:github-actions`, `tech:python`.
- **Knowledge store:** `qualitycoding/agent-knowledge` (A-018).

## Software intake (0.3.2)
- **Target users / platforms:** composers and producers using a DAW; the project owner. Windows 11 x64, macOS
  (Apple silicon and Intel via universal build where the JUCE default allows), Linux x64.
- **Runtime / deployment target:** none (`software.deploys = false`); CI artifacts only.
- **Performance targets:** A-012.
- **Threat model / data sensitivity:** A-015. Data sensitivity: public.
- **Data handling and retention:** plugin state is stored only in the host's project file; nothing is
  transmitted; reference recordings are downloaded at test time into a git-ignored folder and deleted with it.
- **Release channel / versioning:** SemVer `0.y.z`; no tags or releases without G-002.
- **Maintenance:** single maintainer, hobby project; CI must stay green on all three OSes; dependencies pinned
  and updated deliberately (A-017).

## Assumptions

| ID | Topic | Decision (adopted default) | Source |
|---|---|---|---|
| A-001 | Profiles | `software` only; `software.deploys = false`; no technical note or archive record. | intake 0 (restatement) |
| A-002 | Instrument and range | B♭ trumpet, three piston valves. Normal written range F♯3–C6 (concert E3–B♭5, MIDI 52–82). Written C♯6–F6 (concert B5–E♭6, MIDI 83–87) only by overblowing (A-007a) or as an explicit "extended range" note when Overblow ≥ the per-note threshold (D-### in DECISIONS). Pedal tones (1st partial) only by underblow. | intake 1 |
| A-003 | Pitch convention | MIDI input is concert pitch; the UI shows written (B♭ transposition, +2 semitones) and concert note names. | intake 2 |
| A-004 | Synthesis | Physical model, no samples: lip valve (outward-striking one-mass model or the model chosen in research) + modal resonator per valve combination fitted to measured or computed B♭ trumpet input impedance + nonlinear propagation (wave steepening) in the bore for *ff* brassiness. | intake 3 |
| A-005 | Variants | B♭ only; no C-trumpet switch. | intake 4 |
| A-006 | Intonation | In tune by default (the virtual player lips the note in and uses the slide triggers); an *Intonation realism* knob (0–1) restores the natural tendencies (sharp 1-3 and 1-2-3, flat 5th partial, etc.). | intake 5 |
| A-007 | Overblowing | (a) *Overblow* knob, also on a MIDI CC, range −1…+1: 0 = the intended partial; > 0 raises lip tension: brighter, then splits/cracks, then jumps to the next partials of the same valve combination; < 0 drops toward the pedal tones. (b) Lip slurs: legato between two notes with the same valve combination moves through the bore resonances instead of gliding. (c) *Fixed valves* mode: keyswitches hold one of the 8 valve combinations and played notes choose which partial sounds. (d) *Alternate fingerings* toggle: uses the standard alternative where one exists. | intake 6 |
| A-008 | Out of scope v1 | Half-valve effects, multiphonics, flutter tongue, falls/doits, shakes, mutes. | intake 7 |
| A-009 | UI | Programmatically drawn side view of a B♭ trumpet: 3 valves pressed/up, 1st and 3rd slide triggers drawn extended when used, a harmonic-series ladder marking the target and the sounding partial, a caption (written note, concert note, valve combination, partial number), knobs and mode toggles. No generative-AI imagery. | intake 8 |
| A-010 | Playing | Monophonic, last-note priority; velocity → attack (tonguing strength); breath CC2/CC11 → dynamics once received; pitch bend ±2 semitones (lip bend); channel pressure or mod wheel (CC1) → lip vibrato depth; MIDI omni; velocity-0 note-on = note-off. | intake 9 |
| A-011 | Tone controls | Brightness, lip stiffness, breath noise, vibrato rate, vibrato depth, tuning (A4 Hz), intonation realism, output gain (plus Overblow, A-007). | intake 10 |
| A-012 | Performance | ≤ 5 % of one core at 48 kHz / 128-sample blocks (real-time factor ≤ 0.05) on a GitHub-hosted runner; no allocation and no locks on the audio thread; latency only from the oversampling decimator, reported to the host. | intake 12 |
| A-013 | Realism | Objective comparison with TinySOL "Trumpet in C" ordinario recordings (CC BY 4.0, downloaded during tests or CI, never committed) at pp/mf/ff. Thresholds derived from the measured spread of the reference data in a planning spike; attack time measured from the onset. Plus a blind A/B listening gate (G-005) and a fingering-chart/UI check gate (G-004). | intake 13 |
| A-014 | Human test setup | Windows 11 + Reaper. Gate evidence is delivered as rendered WAVs and screenshots on GitHub. | intake 14 |
| A-015 | Threat model | Low: offline plugin, no network at runtime, no personal data. Attack surface: host-supplied state blobs and the embedded resonator data table → validation and fuzz-style tests. | intake 15 |
| A-016 | Licence and repository | Own source Apache-2.0 (README, LICENSE, NOTICE committed to `main` at 3a7e0be). JUCE under AGPLv3 or a JUCE licence; binary distribution out of scope; any tag or release behind G-002. Plan artefacts on the generation branch; implementation on `impl/trumpet-v1`; merging to `main` is the human's decision. | intake 16 |
| A-017 | Maintenance | Solo hobby maintenance; readability and tests over extensibility. Patterns copied from clarinet-vst; no shared library between repositories. | intake 17 |
| A-018 | Knowledge store | `qualitycoding/agent-knowledge`. It does not exist (checked 2026-10-07, HTTP 404). The human was asked to create it; if it still does not exist at S-KNOW, the implementer tries `gh repo create`, and on failure writes the git-bundle fallback (protocol 3.7.2 step 3). | intake 18 |
| A-019 | Backfilled lessons | Problems found in the clarinet-vst, saxophone-vst and chinese-strings runs (which have no `lessons/` directory) are recorded as planner lessons tagged with their source run, applied in this plan, and pushed with this run's lessons at G-003. | intake 19 |
| A-020 | Tokens | Two PATs were pasted in chat. The first is superseded and should be revoked by the human; the second is held only in the planning container's `gh` keyring and is never written to the repository, lessons or knowledge. The implementer uses its own credential (`plan/ENVIRONMENT.md`). | intake 20 |
| A-021 | Execution environment (planning) | Claude Code cloud sandbox, Ubuntu 24.04, 4 vCPU; subagents available with tiers Fable, Opus, Sonnet, Haiku; no sudo, no system pip (pip bootstrapped into a venv, CMake/Ninja from GitHub release binaries, `lessons/L-20261007T150000Z-sandbox-no-pip-no-sudo.md`). | planner (diagnostic) |
| A-022 | CI runners | GitHub-hosted `ubuntu-24.04`, `macos-15`, `windows-2025` (same as clarinet-vst). | planner |
| A-023 | Instrument data provenance | Bore impedance data, lip parameters and fingering/intonation facts come from Tier-1/Tier-2 sources recorded in `research/claims.json`; if no measured impedance for all 8 valve combinations is available, the resonators are computed with a transfer-matrix model of a published trumpet bore profile, and the choice is recorded as a decision. | planner |
| A-024 | Git identity | Commits use `qualitycoding <qualitycoding@users.noreply.github.com>` (as in the sibling repositories). | planner |
