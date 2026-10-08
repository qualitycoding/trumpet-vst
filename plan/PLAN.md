<!-- STATUS: see the header written in Phase 5 below this comment. -->
# Execution plan — trumpet-vst v1

## Conventions
- **Directories:** run `mkdir -p logs` once in S-000; `logs/` is committed.
- **Branch:** run every command from the repository root on the implementation branch
  `impl-20261007T145241Z-trumpet-vst-plan` (created in S-000).
- **Build:** "build" means `cmake --build build -j4`, with the build directory configured as in `plan/ENVIRONMENT.md`.
- **After every step attempt:**
  1. commit the work with the message `S-0xx: <title>`;
  2. append one line to `EXECUTION_LOG.md`: `UTC time | step | attempt | pass/fail/blocked | <sha of commit 1> | note`;
  3. append `{"step":"S-0xx","status":"done|failed|blocked","commit":"<sha of commit 1>","utc":"…"}` to the JSON array in
     `.checkpoints/impl-state.json` (create it as `[]` in S-000);
  4. commit steps 2 and 3 as `S-0xx: record`;
  5. push.
- **Freeze check:** run `bash tests/scripts/verify_freeze.sh` before and after every step; it must print `freeze OK`.
- **Idempotency:** every step is safe to re-run. Completion is detected by its "Done when" checks.
- **Lessons and knowledge:**
  - Whenever you correct your own execution (Rule 11), write a lesson in `lessons/` before continuing.
  - Whenever you learn a new fact about the subject (Rule 12), write a knowledge item in `knowledge/`.
  - Run `python3 research/spikes/schema_check.py` before committing either.
- **Exclusive resources:**
  - Steps that edit `core/src/TrumpetVoice.cpp` or `core/src/VoiceTuning.h` never run concurrently.
  - CI runs are shared: push at most one commit at a time while waiting for CI.

### S-000 Environment & access verification
- Tier: Haiku
- Profile: software
- Depends on: none
- Inputs: generation branch head; `plan/ENVIRONMENT.md`; `HANDOFF.md`
- Actions:
  1. Credentials: the harness provides the token in the environment variable `GH_TOKEN` (gh reads it; never echo it). Run
     `gh auth setup-git`, `git config user.name qualitycoding` and
     `git config user.email qualitycoding@users.noreply.github.com` (A-024).
     Then `git fetch origin && git checkout -b impl-20261007T145241Z-trumpet-vst-plan origin/gen-20261007T145241Z-trumpet-vst-plan`
     (if the branch exists: `git checkout impl-20261007T145241Z-trumpet-vst-plan`). `mkdir -p logs`.
  2. Install the tools per `plan/ENVIRONMENT.md` (the sudo variant if `sudo -n true` succeeds, else the no-sudo variant).
  3. Check access:
     - `gh auth status`;
     - `git push --dry-run origin HEAD:refs/heads/impl-20261007T145241Z-trumpet-vst-plan`;
     - `gh api repos/qualitycoding/trumpet-vst --jq .permissions.push` (must be `true`);
     - `gh api repos/qualitycoding/trumpet-vst/actions/runs --jq '.total_count'` (must succeed).
  4. Check that hosts are reachable with `curl -s -o /dev/null -m 15 -w "%{http_code}" https://<host>/`. Any HTTP
     status (not `000`) counts as reachable; some hosts answer 404 at the root (files.pythonhosted.org). Hosts:
     github.com, api.github.com, codeload.github.com, zenodo.org, pypi.org, files.pythonhosted.org, bootstrap.pypa.io.
     Record each result.
  5. `bash tests/scripts/verify_freeze.sh`.
  6. Create `EXECUTION_LOG.md` with its header line, and `.checkpoints/impl-state.json` containing `[]`.
- Outputs: `EXECUTION_LOG.md`, `.checkpoints/impl-state.json`
- Evidence produced: none
- Done when: every check succeeds and `freeze OK` is printed.
- Checkpoint: the reachability and permission results.
- On failure: write `BLOCKED.md` listing exactly what is missing (credential, permission, host), then halt before any delivery work. Exception: zenodo.org unreachable locally is not blocking (DR-ZENODO).
- Gate: none
- Relevant decisions/claims: D-019, D-022, C-090
- Lessons applied: L-20261007T150000Z-sandbox-no-pip-no-sudo, L-20261007T150100Z-no-credentials-in-command-lines
- Exclusive resources: none

### S-001 Baseline build and red run
- Tier: Sonnet
- Profile: software
- Depends on: S-000
- Inputs: `plan/ENVIRONMENT.md`; `research/spikes/plan_verify_ci.md`
- Actions:
  1. `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release` (add `-DTPT_BUILD_PLUGIN=OFF` if the JUCE Linux packages are not installable), then build.
  2. `ctest --test-dir build --output-on-failure > logs/S-001-red-ctest.txt 2>&1 || true`.
  3. `.venv/bin/python -m pytest tests/python > logs/S-001-red-pytest.txt 2>&1 || true`.
  4. Compare with the planning red run in `plan/ENVIRONMENT.md`:
     - every C++ test case in unit, integration, operational, alloc and perf fails;
     - if the plugin was built: `tpt_plugin_tests` has 1 passing case ("T-026 parameters", guard) and 4 failing;
     - Python results: 3 passed (T-031 guards), 6 skipped (T-029), the rest failing or erroring on `NotImplementedError` or missing S-002/S-005 outputs.
- Outputs: `build/`, `.venv/`, `logs/S-001-red-ctest.txt`, `logs/S-001-red-pytest.txt`
- Evidence produced: none (baseline)
- Done when: the build succeeds and the red pattern matches D-018.
- Checkpoint: red counts.
- On failure: DR-STUB, DR-DEP.
- Gate: none
- Relevant decisions/claims: D-018, D-022
- Lessons applied: L-20261007T150000Z-sandbox-no-pip-no-sudo
- Exclusive resources: none

### S-002 CI workflow and third-party notices
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: D-020, D-021; `.github/workflows/plan-verify.yml` (template for runner setup)
- Actions:
  1. Create `.github/workflows/ci.yml` with the five jobs of D-020 (`freeze`, `core`, `python`, `plugin`, `realism`):
     - `on: [push, pull_request]`, top-level `permissions: contents: read`, and job-level `contents: write` for `realism` only;
     - actions pinned to the D-020 SHAs;
     - Linux packages exactly as in `plan-verify.yml`;
     - the `realism` job sets `TPT_RENDER` and `TINYSOL_DIR`, uses `actions/cache` (key `tinysol-36030a7fe389da86c3419e5ee48e3b7f`), and pushes results to the `ci-results` branch;
     - `::error::` annotations for failures.
  2. Delete `.github/workflows/plan-verify.yml`.
  3. Create `THIRD_PARTY_NOTICES.md` with one row per D-021 component: name, version, licence, URL, use, shipped yes/no.
  4. Push, then `gh run watch` (or poll `gh run list --branch impl-20261007T145241Z-trumpet-vst-plan`).
- Outputs: `.github/workflows/ci.yml`, `THIRD_PARTY_NOTICES.md`
- Evidence produced: T-031 (all four cases)
- Done when: `.venv/bin/python -m pytest tests/python/test_supply_chain.py` passes, and the CI `freeze` job is green.
- Checkpoint: CI run URL.
- On failure: a YAML error → fix it. Never unpin an action, and never add `pull_request_target`.
- Gate: none
- Relevant decisions/claims: D-020, D-021, C-018–C-022, C-097, C-098
- Lessons applied: L-20261007T150900Z-windows-sha256sum-backslash (keep the frozen script; workflow-level fixes only)
- Exclusive resources: CI

### S-003 Pitch, valves, fingering, note resolution
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: `core/include/tpt/{Pitch,Valves,Fingering}.h`; `tests/fixtures/trumpet_fingerings.txt`; D-001–D-003, D-023
- Actions:
  1. Implement `core/src/Pitch.cpp`, `core/src/Valves.cpp` and `core/src/Fingering.cpp`. The table is a static array transcribed from the fixture in fixture order; generate it with `tools/gen_fingering_table.py` (reads the fixture, prints the C++ array), never by hand. `resolveNote` follows the header contract exactly.
  2. Build; run `build/tests/tpt_unit_tests "[T-001],[T-002],[T-003],[T-004],[T-005]"`.
- Outputs: `core/src/{Pitch,Valves,Fingering}.cpp`, `tools/gen_fingering_table.py`
- Evidence produced: T-001, T-002, T-003, T-004, T-005
- Done when: those tags pass.
- Checkpoint: passing tags.
- On failure: DR-T003. Never edit the fixture.
- Gate: none
- Relevant decisions/claims: D-001, D-002, D-003, C-056–C-065
- Lessons applied: L-20261007T150300Z-hand-typed-frozen-constant (generated table)
- Exclusive resources: none

### S-004 Analysis utilities (C++ and Python) and estimator cross-check
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: `core/include/tpt/Analysis.h`; `tools/realism/metrics.py`; `research/spikes/{ref_spread,regime_map}.py`; `research/spikes/lipsim.cpp`; `research/spikes/table_draft.json`; `research/spikes/regime_map_result.json`; D-017
- Actions:
  1. Implement YIN, `harmonicLevelsDb`, `spectralCentroidHz`, `nearestPartial` and `rmsDb` in `core/src/Analysis.cpp`, with a self-written radix-2 FFT (no new dependency), exactly per the header contracts.
  2. Implement `tools/realism/metrics.py` with the same definitions as `research/spikes/ref_spread.py` (`frames_db`, `steady_segment`, `harmonic_levels_db`, `spectral_centroid_hz`, `onset_ms`, `harmonic_mad_db`).
  3. Cross-check (R5 question 5), testing the shipped C++ estimator:
     - create `tools/render/CMakeLists.txt` with a target `tpt_f0` (`tools/render/f0.cpp`, links `tpt_core`). It reads a raw float32 mono file plus a sample rate and prints `estimateF0` for the last 0.5 s;
     - add `add_subdirectory(tools/render)` after `add_subdirectory(core)` in the root `CMakeLists.txt`;
     - write `tools/render/yin_crosscheck.py`, which re-renders each row of `research/spikes/regime_map_result.json`
       with `research/spikes/lipsim.cpp` (built with `g++ -O2 -std=c++20`), calling it exactly as `sim()` in
       `research/spikes/regime_map.py` does: poles from `research/spikes/table_draft.json` (state of the row),
       `fl = ratio(n)·f_res`, `H = h0(n)`, `Ql=20 mu=9 b=12e-3 attack=0.003 yinit=0 fscale=1.0 fmax=2000 fmin=40`,
       `pm = 2.5·pth(n)`, `dur=2.0`, `wav=<file> raw=1` (raw float32, no header);
       set `LIPSIM` and `TMPDIR_SPIKE` to paths of your own;
     - for each row, take the nearest partial of the `tpt_f0` result and compare it with the row's `mf_partial`;
     - write `logs/S-004-yin-crosscheck.md`.
  4. Run `build/tests/tpt_unit_tests "[T-010]"` and `.venv/bin/python -m pytest tests/python/test_realism_metrics.py`.
- Outputs: `core/src/Analysis.cpp`, `tools/realism/metrics.py`, `tools/render/{CMakeLists.txt,f0.cpp,yin_crosscheck.py}`, `CMakeLists.txt`, `logs/S-004-yin-crosscheck.md`
- Evidence produced: T-010, T-028
- Done when:
  - T-010 and T-028 pass;
  - the cross-check covers the 57 rows (56 fixture fingerings plus the dropped alternate written 80); rows whose
    `mf_partial` is 0 (written 88: no sound at mf) are excluded; agreement on all remaining rows except at most 2, each
    explained in the log (an octave error is a YIN defect → fix it).
- Checkpoint: cross-check agreement count.
- On failure: f0 error > 0.5 c → check the parabolic interpolation and the 1.0 s window. A YIN octave error → lower the absolute threshold to 0.08 (a private constant; log the change).
- Gate: none
- Relevant decisions/claims: D-017, C-073
- Lessons applied: L-20261008T010500Z-circular-fft-filter-fakes-onsets (zero-pad any spectral filtering)
- Exclusive resources: none

### S-005 Resonator tools (Python) and the table
- Tier: Opus
- Profile: software
- Depends on: S-001
- Inputs: `tools/resonator/*.py` stubs; `research/spikes/{tmm_trumpet,bore_fit,make_table}.py`; `tests/fixtures/freour2022_open_modes.txt`; D-004
- Actions:
  1. Implement `tools/resonator/tmm.py`:
     - port `tmm_trumpet.py` (cone via the spherical-wave fundamental solutions; losses as in the spike; `lossless=True` → Γ = jω/c);
     - `peaks` as in the spike;
     - `complex_modal_fit`: poles initialised from the peaks, complex residues by linear least squares, then `scipy.optimize.least_squares` on the real and imaginary parts of Z with weights 1/|Z|, f ≤ 1.1·f_max. Enforce Re(s) < 0 and Re(R) > 0, and sort by Im.
  2. Implement `tools/resonator/bore_fit.py`: port `research/spikes/bore_fit.py`, but compare the measured poles with the poles of `complex_modal_fit` (C-099).
  3. Implement `tools/resonator/generate_table.py` exactly per D-004: loops by bisection, 14 modes, calibration ratios, trigger states, `natural_dev_cents`, the radiation fit, deterministic output, and `--keep-calibration`. `fscale` defaults to 1.0 for new notes.
  4. Run: `.venv/bin/python -m tools.resonator.generate_table --fingerings tests/fixtures/trumpet_fingerings.txt --out data/trumpet_resonators.json`.
  5. Run: `.venv/bin/python -m pytest tests/python/test_resonator_tools.py`.
- Outputs: `tools/resonator/{tmm,bore_fit,generate_table}.py`, `data/trumpet_resonators.json`
- Evidence produced: T-030
- Done when: T-030 passes, and `data/trumpet_resonators.json` is committed with its SHA-256 recorded in the checkpoint.
- Checkpoint: table SHA-256, bore-fit cents errors, loop lengths, radiation fit RMS error.
- On failure: DR-T007. The bore fit does not reach ≤ 6 c → widen the initial parameter span (never the tolerance). The radiation fit fails at 6 sections → `BLOCKED.md` with the fit report.
- Gate: none
- Relevant decisions/claims: D-004, C-076, C-077, C-079, C-099
- Lessons applied: L-20261007T150300Z-hand-typed-frozen-constant, L-20261007T150700Z-regime-selection-per-note (via S-009), L-20261008T010500Z-circular-fft-filter-fakes-onsets
- Exclusive resources: none

### S-006 Resonator table parsing, validation and embedding
- Tier: Sonnet
- Profile: software
- Depends on: S-003, S-005
- Inputs: `core/include/tpt/ResonatorTable.h`; D-004; `tests/fixtures/resonators_valid.json`
- Actions:
  1. Implement `core/src/ResonatorTable.cpp` with nlohmann/json:
     - check the size before parsing;
     - `parse(..., nullptr, false)`;
     - apply every D-004 limit;
     - wrap every exception as `ParseError`.
  2. Embed the table:
     - in `core/CMakeLists.txt`, at configure time, read `data/trumpet_resonators.json` with `file(READ ... HEX)`;
     - write `${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp`, defining `embeddedResonatorJson()` as a byte array with a terminating 0;
     - add `CMAKE_CONFIGURE_DEPENDS` on the JSON;
     - remove the stub definition.
  3. Run `tpt_unit_tests "[T-006],[T-007]"`.
- Outputs: `core/src/ResonatorTable.cpp`, `core/CMakeLists.txt`, `core/embed_table.cmake`
- Evidence produced: T-006, T-007
- Done when: T-006 and T-007 pass.
- Checkpoint: passing tags.
- On failure: DR-T007.
- Gate: none
- Relevant decisions/claims: D-004, A-015
- Lessons applied: L-20261007T150800Z-host-facing-accessors-missing (no new accessors expected; any addition is additive and logged)
- Exclusive resources: none

### S-007 Lip model and reference integrator
- Tier: Opus
- Profile: software
- Depends on: S-004, S-006
- Inputs: `core/include/tpt/{LipModel,ReferenceSim}.h`; `research/spikes/lipsim.cpp`; D-005, D-006
- Actions:
  1. Implement `core/src/LipModel.cpp` with exactly the D-005 constants.
  2. Implement `core/src/ReferenceSim.cpp` as a line-by-line port of the `lipsim.cpp` main loop (implicit flow, bilinear modes, ramps, attack, `yInit`, `fscale`). Argument checks per the header.
  3. Run `tpt_unit_tests "[T-008]"` and `tpt_integration_tests "[T-009]"`.
- Outputs: `core/src/{LipModel,ReferenceSim}.cpp`
- Evidence produced: T-008, T-009
- Done when: T-008 and T-009 pass.
- Checkpoint: measured f0 values.
- On failure: DR-T009.
- Gate: none
- Relevant decisions/claims: D-005, D-006, C-036, C-044, C-053, C-075
- Lessons applied: L-20261007T150600Z-control-mapping-formulas-unverified (port the spike-verified formulas exactly)
- Exclusive resources: none

### S-008 Real-time voice core
- Tier: Opus
- Profile: software
- Depends on: S-007
- Inputs: `core/include/tpt/TrumpetVoice.h`; D-005–D-008, D-012, D-013, D-023
- Actions:
  1. Create `core/src/VoiceTuning.h` holding every non-frozen constant of D-005, D-007, D-008, D-009, D-012 and D-023, each with a comment citing its decision.
  2. Implement `TrumpetVoice::Impl`:
     - oversampling and decimators (D-006);
     - the control-rate grid;
     - lips, flow and modes;
     - the attack (D-007) and dynamics map (D-008) with the D-023 floors;
     - the output stage (D-012);
     - the UI atomics and the partial tracker (D-013);
     - idle detection and the safety reset;
     - the note stack (16 notes) with last-note priority.
     Also implement `VoiceParameters::clamped()`, `latencySamples()` and `soundingHz()`.
  3. Run `tpt_operational_tests` and `tpt_alloc_tests`.
- Outputs: `core/src/TrumpetVoice.cpp`, `core/src/VoiceTuning.h` (+ private helpers in `core/src/`)
- Evidence produced: T-023, T-024 (alloc part), T-025
- Done when: `tpt_operational_tests` and `tpt_alloc_tests` pass, except that the single pitch assertion `std::fabs(1200 * std::log2(f0 / tpt::equalTemperedHz(67))) <= 15.0` in the case "T-023 sample rates and latency (T-025)" may still fail (it needs calibration, S-009); every other assertion of that case passes (inspect the Catch2 output).
- Checkpoint: test status.
- On failure: DR-ALLOC. Non-finite output → the D-006 safety reset; investigate the flow-solve branch.
- Gate: none
- Relevant decisions/claims: D-005–D-008, D-012, D-013, D-023, C-089, C-091, C-094
- Lessons applied: L-20261007T150600Z-control-mapping-formulas-unverified, L-20261007T150800Z-host-facing-accessors-missing
- Exclusive resources: `core/src/TrumpetVoice.cpp`, `core/src/VoiceTuning.h`

### S-009 Tuning calibration
- Tier: Sonnet
- Profile: software
- Depends on: S-008
- Inputs: D-011; `data/trumpet_resonators.json`
- Actions:
  1. (`tools/render/CMakeLists.txt` exists from S-004.)
  2. Add `tools/render/calibrate.cpp` → target `tpt_calibrate` (links `tpt_core` and nlohmann_json). It loads the JSON, calibrates every note per D-011 (with sustain gating), and rewrites `fscale` in place, formatted with `%.10g` and otherwise preserving the file as in `generate_table` (D-004 number formatting).
  3. Run `build/tools/render/tpt_calibrate data/trumpet_resonators.json`; rebuild (this re-embeds the table).
  4. Run `tpt_integration_tests "[T-012],[T-013]"` and `tpt_unit_tests "[T-007]"`.
- Outputs: `tools/render/{CMakeLists.txt,calibrate.cpp}`, `data/trumpet_resonators.json`
- Evidence produced: T-012, T-013
- Done when: those tags pass.
- Checkpoint: maximum |cents| per note, and the fscale range.
- On failure: DR-CAL, DR-REGIME.
- Gate: none
- Relevant decisions/claims: D-011, C-091, C-092
- Lessons applied: L-20261007T150700Z-regime-selection-per-note
- Exclusive resources: `data/trumpet_resonators.json`

### S-010 Expression and MIDI behaviour
- Tier: Sonnet
- Profile: software
- Depends on: S-009
- Inputs: D-011, D-014
- Actions:
  1. Implement in the voice:
     - breath and velocity handling;
     - legato note stack behaviour;
     - the full `fscale_eff` of D-011: Intonation realism with `naturalDevCents`, A4 tuning, pitch bend, vibrato;
     - all-notes-off;
     - out-of-range note-ons ignored.
  2. Run `tpt_integration_tests "[T-014],[T-019]"` plus every previously green tag.
- Outputs: `core/src/TrumpetVoice.cpp`
- Evidence produced: T-014, T-019
- Done when: T-014 and T-019 pass with no regression.
- Checkpoint: test status.
- On failure: DR-DEFAULT.
- Gate: none
- Relevant decisions/claims: D-011, D-014
- Lessons applied: none
- Exclusive resources: `core/src/TrumpetVoice.cpp`

### S-011 Overblow, lip slurs, valve slurs, Fixed valves, alternates
- Tier: Opus
- Profile: software
- Depends on: S-010
- Inputs: D-003, D-009, D-010; C-083, C-084, C-093
- Actions:
  1. Implement in the voice:
     - the D-009 mapping (hysteresis, cap, 20 ms register ramps, brightening, CC16 precedence);
     - D-010 lip slurs and valve-change interpolation;
     - D-003 keyswitches and fixed-valves fscale;
     - `useAlternates`.
  2. Run `tpt_integration_tests "[T-015],[T-016],[T-017],[T-018],[T-020],[T-021],[T-022]"` plus every previously green tag.
- Outputs: `core/src/TrumpetVoice.cpp`, `core/src/VoiceTuning.h`
- Evidence produced: T-015, T-016, T-017, T-018, T-020, T-021, T-022
- Done when: those pass with no regression. T-021/T-022 failures may wait for S-016 under DR-REAL; record them in the checkpoint.
- Checkpoint: test status and final constants.
- On failure: DR-OVERBLOW, DR-VALVE.
- Gate: none
- Relevant decisions/claims: D-003, D-009, D-010, D-013
- Lessons applied: L-20261007T150600Z-control-mapping-formulas-unverified
- Exclusive resources: `core/src/TrumpetVoice.cpp`, `core/src/VoiceTuning.h`

### S-012 State persistence
- Tier: Sonnet
- Profile: software
- Depends on: S-008
- Inputs: `core/include/tpt/State.h`; D-015
- Actions:
  1. Implement `core/src/State.cpp` with nlohmann/json:
     - check the size first;
     - `parse(text, nullptr, false)`;
     - for each field: `is_number()` for floats, `is_boolean()` for bools;
     - apply `clamped()`;
     - catch everything and return nullopt.
  2. Run `tpt_unit_tests "[T-011]"`.
- Outputs: `core/src/State.cpp`
- Evidence produced: T-011
- Done when: T-011 passes.
- Checkpoint: test status.
- On failure: DR-DEFAULT.
- Gate: none
- Relevant decisions/claims: D-015, A-015
- Lessons applied: none
- Exclusive resources: none

### S-013 Plugin processor
- Tier: Sonnet
- Profile: software
- Depends on: S-011, S-012
- Inputs: `plugin/src/PluginProcessor.*`; D-010–D-016
- Actions:
  1. Implement the processor:
     - **Voice:** create `tpt::TrumpetVoice` in the constructor from `embeddedResonatorJson()`, parsed once.
     - **prepareToPlay:** `voice.prepare(fs, max block)` and `setLatencySamples(voice.latencySamples())`.
     - **processBlock:** `ScopedNoDenormals`; read the APVTS values into `VoiceParameters`; split the block at MIDI events and dispatch per D-014; render mono into channel 0 and copy it to every other channel.
     - **State:** per D-015.
     - **`currentUiState()`:** returns `voice.uiState()`.
  2. Build with the plugin; run `xvfb-run -a build/plugin/tpt_plugin_tests` (all except the editor case).
- Outputs: `plugin/src/PluginProcessor.cpp`
- Evidence produced: T-026 (parameters, sound, keyswitch and state cases)
- Done when: those cases pass, locally or in the CI `plugin` job.
- Checkpoint: test status and CI run URL.
- On failure: DR-DEFAULT.
- Gate: none
- Relevant decisions/claims: D-013–D-016
- Lessons applied: L-20261007T150800Z-host-facing-accessors-missing
- Exclusive resources: CI

### S-014 Editor, trumpet drawing, layout, G-004 bundle
- Tier: Sonnet
- Profile: software
- Depends on: S-013
- Inputs: D-013; `core/include/tpt/Layout.h`
- Actions:
  1. Implement `core/src/Layout.cpp`: the layout per D-013 and `captionText` per the header. Run `tpt_unit_tests "[T-033]"`.
  2. Implement `TrumpetView`: the drawing per D-013, and `setState` with repaint on change.
  3. Implement the editor:
     - 9 rotary knobs and 2 toggles with APVTS attachments;
     - a caption label;
     - the 60 Hz timer and `refreshFromProcessor()`.
  4. Add `tools/render/snapshot_ui.cpp` → target `tpt_ui_snapshots` (in `plugin/CMakeLists.txt`, links TrumpetVST).
     - It does not run the voice: for each image it builds a `UiState` from `resolveNote` (soundingPartial = target
       partial, sounding = true, trigger per D-013 at realism 0) and calls `TrumpetView::setState`, then
       `createComponentSnapshot` of the editor (960×600) and writes PNGs to `renders/G-004/` (git-ignored).
     - The image list and names are in `plan/GATES.md` (G-004).
     - Produce the bundle with the CI job `gate-evidence` (D-020), which builds the plugin, runs `tpt_ui_snapshots` under
       `xvfb-run -a`, and pushes the PNGs plus `SHA256SUMS` to the orphan branch `evidence/G-004`. (With sudo locally you
       may run the same script `tools/render/push_evidence.sh G-004 renders/G-004`.) Copy only `SHA256SUMS` to
       `gates/G-004/` on the implementation branch.
  5. Run all T-026 cases. Write `GATE-G-004.md` per `plan/GATES.md`.
- Outputs: `core/src/Layout.cpp`, `plugin/src/{TrumpetView,PluginEditor}.cpp`, `tools/render/{snapshot_ui.cpp,push_evidence.sh}`, `plugin/CMakeLists.txt`, `.github/workflows/ci.yml` (job `gate-evidence`), `GATE-G-004.md`, `gates/G-004/SHA256SUMS`
- Evidence produced: T-033, T-026 (editor case)
- Done when: T-033 and all of T-026 pass, the G-004 bundle is pushed, and `GATE-G-004.md` is committed.
- Checkpoint: gate issued.
- On failure: DR-DEFAULT.
- Gate: **G-004**
- Relevant decisions/claims: D-002, D-013, C-061–C-065
- Lessons applied: L-20261007T151000Z-large-evidence-off-impl-branch
- Exclusive resources: none

### S-015 Offline renderer and TinySOL tooling
- Tier: Sonnet
- Profile: software
- Depends on: S-011
- Inputs: D-017; `tools/realism/{tinysol,compare_tinysol}.py` stubs; C-072, C-096
- Actions:
  1. Add `tools/render/main.cpp` → target `tpt_render` (core only):
     - `tpt_render --note <concert> --velocity <0..1> --seconds <s> --fs <hz> --out <file.wav> [--param <name>=<value> ...]`
       renders one held note (`<name>` = a `VoiceParameters` field name);
     - `tpt_render --events <file.json> --fs <hz> --out <file.wav>` renders a timed event list:
       `{"seconds": 6.0, "params": {...}, "events": [{"t": 0.0, "type": "noteOn", "note": 58, "velocity": 0.6},
       {"t": 1.0, "type": "noteOff", "note": 58}, {"t": 0.5, "type": "param", "name": "overblow", "value": 0.6},
       {"t": 0.2, "type": "breath|bend|vibrato|overblowCC", "value": 0.5}, {"t": 0.0, "type": "keyswitch", "note": 29}]}`;
       events are applied at the first sample at or after `t`, in list order;
     - writes a mono float32 WAV;
     - exits 0 on success, 2 on bad arguments or a malformed event file.
  2. Implement `tinysol.download` (Zenodo REST API, record 3685367, streamed download, md5 verify, partial extraction, idempotent) and `trumpet_notes`.
  3. Implement `compare_tinysol.main` per its docstring, using `metrics.py`.
  4. Download to `reference-data/tinysol` (git-ignored). Record the md5 values, the note count (82) and the attribution in `data/REFERENCE_DATA.md`.
- Outputs: `tools/render/main.cpp`, `tools/render/CMakeLists.txt`, `tools/realism/{tinysol,compare_tinysol}.py`, `data/REFERENCE_DATA.md`
- Evidence produced: none directly (enables T-029)
- Done when: `TPT_RENDER=build/tools/render/tpt_render TINYSOL_DIR=reference-data/tinysol .venv/bin/python -m pytest tests/python/test_realism_vs_tinysol.py` reaches the threshold assertions (pass or fail), and `test_t029_coverage` and `test_t029_tinysol_selection` pass.
- Checkpoint: note count.
- On failure: DR-ZENODO.
- Gate: none
- Relevant decisions/claims: D-017, C-072, C-096
- Lessons applied: L-20261008T010500Z-circular-fft-filter-fakes-onsets
- Exclusive resources: none

### S-016 Realism calibration and G-005 bundle
- Tier: Opus
- Profile: software
- Depends on: S-014 (with G-004 answered `proceed` or `fix-drawing` resolved), S-015
- Inputs: D-007, D-008, D-012, D-017, DR-REAL; `core/src/VoiceTuning.h`
- Actions:
  1. Run T-029 (locally or in the CI `realism` job), T-021 and T-022. Write `logs/S-016-round-<k>.md` with the per-dynamic metrics and the 10 worst notes, split by register (p2–p3, p4–p6, p8+).
  2. Adjust only the non-frozen constants listed in DR-REAL. After each round, re-run every frozen test (no regression). At most 5 rounds.
  3. Build the G-005 bundle per `plan/GATES.md`:
     - `tools/realism/make_g005_bundle.py --tinysol <download dest> --render <tpt_render> --out renders/G-005`, using
       `tpt_render --events` with the event files `tools/realism/demos/*.json` (contents in `plan/GATES.md`) and the
       blind-pair recipe in `plan/GATES.md`; plus `ATTRIBUTION.txt` (copy of the TinySOL section of `ATTRIBUTION.md`);
     - run it in the CI job `gate-evidence` (D-020), which pushes `renders/G-005` to the orphan branch
       `evidence/G-005`, or locally with `tools/render/push_evidence.sh G-005 renders/G-005`.
     Write `GATE-G-005.md`.
- Outputs: `core/src/VoiceTuning.h`, `logs/S-016-*.md`, `tools/realism/make_g005_bundle.py`, `tools/realism/demos/*.json`, `GATE-G-005.md`
- Evidence produced: T-029, T-021, T-022
- Done when: T-029, T-021 and T-022 pass, or 5 rounds are exhausted; and the G-005 bundle is pushed.
- Checkpoint: round number and metrics.
- On failure: DR-REAL.
- Gate: **G-005**
- Relevant decisions/claims: D-007, D-008, D-012, D-017, C-085, C-089, C-094, C-096
- Lessons applied:
  - L-20261007T150400Z-threshold-not-checked-against-reference-spread (thresholds are frozen; never edit them);
  - L-20261007T150500Z-linear-bore-realism-ceiling;
  - L-20261007T151000Z-large-evidence-off-impl-branch.
- Exclusive resources: `core/src/VoiceTuning.h`, CI

### S-017 Performance and plugin validation on all platforms
- Tier: Sonnet
- Profile: software
- Depends on: S-016 (with G-005 answered `proceed` or `proceed-with-rescope`)
- Inputs: CI jobs `core` and `plugin`
- Actions:
  1. Push. Get CI `core` (including `ctest -L perf`) and `plugin` (pluginval strictness 10; auval on macOS) green on all three OSes.
  2. Run `pip-audit -r tools/requirements.lock` (DR-SECURITY).
- Outputs: fixes as needed
- Evidence produced: T-024 (perf), T-026, T-027 on three OSes
- Done when: the CI jobs `freeze`, `core`, `python`, `plugin` and `realism` are green on the same commit — except, when the human answered G-005 with `proceed` or `proceed-with-rescope` while T-029/T-021/T-022 were red (`GATE-G-005.RESPONSE.md`), those named tests may stay red: they are listed as accepted failures in `REPORT.md` (the frozen tests are not changed or skipped).
- Checkpoint: CI run URL.
- On failure: DR-PERF, DR-PLUGINVAL, DR-SECURITY.
- Gate: none
- Relevant decisions/claims: D-020, C-009–C-015, C-025
- Lessons applied: L-20261007T150900Z-windows-sha256sum-backslash
- Exclusive resources: CI

### S-018 Final verification and report
- Tier: Sonnet
- Profile: software
- Depends on: S-017
- Inputs: everything
- Actions:
  1. `bash tests/scripts/verify_freeze.sh`; then a full local run of every suite available locally.
  2. Update the README "Status" and "Building" sections (no claims beyond the evidence).
  3. Write `REPORT.md`: tests by ID with status, the SC table, deviations, gate outcomes, open risks, and what was not verified (a real DAW host test, unless the human did one).
  4. Push.
- Outputs: `README.md`, `REPORT.md`
- Evidence produced: every T-ID green on one commit (CI link)
- Done when: everything is green apart from the accepted failures recorded at G-005, and `REPORT.md` is pushed.
- Checkpoint: final.
- On failure: DR-DEFAULT.
- Gate: none (merging, tags and releases are G-002, outside this plan)
- Relevant decisions/claims: all
- Lessons applied: none
- Exclusive resources: none

### S-RETRO Retrospective
- Tier: Opus (fresh context); Haiku collects the digest
- Profile: software
- Depends on: S-000 … S-018 (every step before it)
- Inputs: `EXECUTION_LOG.md`, `git log`, `DEVIATIONS.md`, `BLOCKED.md`, `TEST_CHALLENGE.md`, `GATE-*.md`, the CI history (`gh run list`), `research/PRIOR_KNOWLEDGE.md`, `lessons/`, `knowledge/`
- Actions:
  1. A Haiku agent collects every input into `logs/retro-digest.md` (without subagent support: do it yourself, then start a fresh section of reasoning for step 2 and log the substitution in `EXECUTION_LOG.md`).
  2. A fresh Opus agent answers the four questions of protocol 3.7.1 (`plan/PROTOCOL_EXTRACTS.md`) and writes `RETROSPECTIVE.md`, linking every finding to an `L-` or `K-` file.
  3. Add a lesson (Appendix A) for every error or overlooked step not yet recorded, and a knowledge item (Appendix B) for every missing fact.
  4. Run `python3 research/spikes/schema_check.py`.
- Outputs: `RETROSPECTIVE.md`, `logs/retro-digest.md`, new `lessons/L-*.md`, new `knowledge/K-*.md`
- Evidence produced: none
- Done when: every finding links to an `L-` or `K-` file, and the schema check exits 0.
- Checkpoint: counts of lessons and knowledge items.
- On failure: fix the entries until the schema check passes.
- Gate: none
- Relevant decisions/claims: none
- Lessons applied: all loaded lessons (`research/PRIOR_KNOWLEDGE.md`)
- Exclusive resources: none

### S-KNOW Lessons & knowledge push
- Tier: Haiku
- Profile: software
- Depends on: S-RETRO
- Inputs: `lessons/`, `knowledge/`; A-018; `plan/PROTOCOL_EXTRACTS.md` (3.7.2, Appendix A–C)
- Actions:
  1. Write `GATE-G-003.md` per `plan/GATES.md` and halt at G-003.
  2. On `push-to-proposed` or `push-to: <repo>`:
     - clone the target; create it if missing (`gh repo create qualitycoding/agent-knowledge --private`) with `TAXONOMY.md`, `lessons/`, `knowledge/` and the two `INDEX.md` files;
     - copy each lesson to `lessons/<first tech: or domain: tag>/` and each item to `knowledge/<first subject: tag>/`;
     - add any new tags to `TAXONOMY.md`;
     - regenerate both `INDEX.md` files (tag → IDs, titles or statements, severity);
     - run the schema check in the target; commit; push.
     - If the push is rejected: pull, regenerate the indexes and retry, up to 3 times.
  3. On `do-not-push`: finish.
  4. If the target is unreachable or no response arrives: `git bundle create knowledge-20261007T145241Z-trumpet-vst-plan.bundle <lessons and knowledge commits>`, and report the path.
- Outputs: the knowledge-store commit, or the bundle
- Evidence produced: none
- Done when: per protocol 3.7.2 step 4.
- Checkpoint: the push commit or the bundle path.
- On failure: the bundle fallback.
- Gate: **G-003**
- Relevant decisions/claims: A-018
- Lessons applied: L-20261007T150100Z-no-credentials-in-command-lines (secret scan via the schema check)
- Exclusive resources: none
