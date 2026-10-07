# Question tree (R1, updated each round)

Each leaf names what it informs (D = decision, T = test, S = plan step, R = risk). Status: open / answered (claim IDs)
/ pruned. Only the `software` branch applies (PROFILE.md); acoustics questions are engineering questions about the
model the software implements.

## E — Engineering (toolchain, platform, supply chain)
- **Q-E1** Latest stable JUCE release, its commit SHA, licence (AGPLv3?), minimum CMake, Linux deps → D-ENV, T-SUPPLY, S-000.
- **Q-E2** Catch2 v3 and nlohmann/json latest release SHAs → D-ENV, T-SUPPLY.
- **Q-E3** pluginval latest release, per-OS zip SHA-256, CLI flags (strictness 10, in-process) → T-PLUGINVAL.
- **Q-E4** VST3 SDK licence as bundled by the pinned JUCE (MIT since 3.8?) → THIRD_PARTY_NOTICES, A-016.
- **Q-E5** GitHub Actions to use (checkout, setup-python, upload-artifact, cache) latest versions + commit SHAs;
  runner images `ubuntu-24.04`, `macos-15`, `windows-2025` available; `auval` on macos-15 → D-CI, T-SUPPLY.
- **Q-E6** Can the stub plugin be built and validated in this sandbox without sudo (local `dpkg -x` prefix), or
  only in CI? → D-ENV, ENVIRONMENT.md, S-000.
- **Q-E7** Does the token allow pushing `.github/workflows/*` (workflows permission)? → S-002, implementer credential.
- **Q-E8** Real-time cost of the chosen voice (modes × oversampling) on a CI-class core → A-012, T-PERF.
- **Q-E9** JUCE headless testing of an AudioProcessor/editor (xvfb on Linux; `juce::ScopedJuceInitialiser_GUI`)
  → T-PLUGIN.

## A — Acoustics model (what the core implements)
- **Q-A1** Lip-valve model for brass: equations, outward-striking one-mass parameters (mass, Q, area, rest opening,
  width), with exact locators → D-LIP, T-LIP.
- **Q-A2** Regime (partial) selection: for a given valve combination, which lip resonance frequency (relative to
  the bore resonances) makes the model sound partial n? Is there hysteresis? → D-OVERBLOW, T-PARTIAL, T-OVERBLOW.
- **Q-A3** Can the model sound the pedal (privileged) tone, partial 1? → A-002, D-OVERBLOW, T-UNDERBLOW.
- **Q-A4** B♭ trumpet bore: published geometry (mouthpiece, leadpipe, cylinder, bell) and valve-loop lengths
  → D-BORE, tools/resonator.
- **Q-A5** Measured input-impedance resonance frequencies of a B♭ trumpet (open and valved) to validate the
  computed bore → T-BORE.
- **Q-A6** Nonlinear propagation (brassiness): algorithm suitable for real time (Vergez & Rodet 2000 / Msallam
  2000), parameters, where in the signal path → D-NLP, T-BRASS.
- **Q-A7** Radiation / mouthpiece-to-outside transfer function (bell high-pass) → D-RAD, T-TIMBRE.
- **Q-A8** Typical blowing pressures pp…ff and their relation to MIDI velocity / breath CC → D-EXPR.
- **Q-A9** Natural intonation tendencies of the B♭ trumpet per partial and valve combination (cents) →
  A-006, T-INTONATION.
- **Q-A10** Does a modal (linear) bore + lip model + output-side nonlinear propagation reach the realism thresholds
  (spike vs TinySOL, L-…150500)? → D-SYNTH, A-013, R-REAL.

## F — Fingering and UI
- **Q-F1** Standard B♭ trumpet fingering chart, written F♯3–F6, with the partial each note uses (≥ 2 independent
  sources) → fixture, T-FINGERING.
- **Q-F2** Standard alternate fingerings and which are "standard" → D-ALT, T-ALT.
- **Q-F3** When are the 1st/3rd slide triggers used (1-3, 1-2-3 low notes; 1st-valve trigger on 5th-partial
  notes?) → D-TRIGGER, T-UI.
- **Q-F4** Valve semitone lowering: 2 = 1, 1 = 2, 3 = 3 semitones (combination sharpness) → D-BORE, A-006.

## R — Reference data
- **Q-R1** TinySOL trumpet: Zenodo record, file naming, licence, note list, dynamics, md5 → D-REF, T-REALISM.
- **Q-R2** Reference spread of each realism metric (harmonic levels, centroid, onset attack) per dynamic; coverage
  by the reference median under candidate thresholds (L-…150400) → A-013 thresholds, T-REALISM.
- **Q-R3** Trumpet in C vs B♭: is the C-trumpet timbre a valid reference for a B♭ model (bore differences)? →
  A-013, R-REF.

## Pruned
- CLAP, AAX, mutes, half-valve: out of scope (A-008, A-016).
