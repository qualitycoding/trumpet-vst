# Pre-mortem round 1 (planning protocol Phase 4)

- Reviewer: fresh-context pre-mortem agent, not the plan author. Read-only review, except for this file.
- Plan head reviewed: `1b60fbb` on `gen-20261007T145241Z-trumpet-vst-plan`. `bash tests/scripts/verify_freeze.sh`
  printed `freeze OK`.
- Inputs read: `HANDOFF.md`, `plan/*.md`, `premortem/RISK_REGISTER.md` (R-001..R-009), `research/PRIOR_KNOWLEDGE.md`
  and all 13 `lessons/`, `research/claims.json` (99 claims), `research/rounds/*.md`, the frozen tests in `tests/`, the
  CMake files, the plugin and core headers, and the spikes used below.
- Independent re-runs, all in the session scratchpad. Nothing was committed.
  - **TMM short-frustum check (R-009 item 1).** This is a port of `research/spikes/tmm_trumpet.py` with the T-030 air
    constants. It ran `test_t030_cone_vs_staircase` with and without the skip. The venv came from
    `tools/requirements.lock` (numpy 2.5.3, scipy 1.18.1).
  - **20 s sustain re-run of the spike model.** This uses `research/spikes/lipsim.cpp` and `table_draft.json`, with the
    `regime_map_result.json` fscale. It covers all 57 rows at pp/mf/ff, using the D-007/D-008 pressure schedule:
    - pp is 1.3 → 0.8 × threshold;
    - mf is 2.5;
    - ff is 4.8, with accent 1.5;
    - the D-023 floors apply.

---

## 1. Incident report (simulation: 6 months after handoff)

**Title:** Trumpet VST3 crashes REAPER when projects load. The owner cannot produce a fixed build because every CI
workflow is red.

**Timeline (simulated)**
1. **Handoff, month 0.**
   - S-018 closed with every frozen test green on one commit, apart from the T-029 pp harmonic-MAD failure accepted at
     G-005 (R-001).
   - `REPORT.md` says, as planned (PLAN.md:513), "not verified: a real DAW host test".
   - No step produced an installable Windows binary. D-020 uploads only logs and `tpt_render`
     (DECISIONS.md:344), and G-002 forbids uploading binaries "anywhere" (GATES.md:33).
   - The owner (Windows 11 + REAPER, A-014) had only ever judged screenshots and offline `tpt_render` WAVs at
     G-004/G-005. The processor and editor had never run inside REAPER.
2. **Month 0, week 2.**
   - The owner did not want to install VS 2026 and fetch JUCE locally. He added an `upload-artifact` step for the
     VST3 to the `plugin` job and installed the `windows-2025` build.
   - That build links the MSVC runtime dynamically. Neither `CMakeLists.txt` nor `plugin/CMakeLists.txt` sets
     `CMAKE_MSVC_RUNTIME_LIBRARY`, and it was built with the VS 2026 (v14.5x) toolset (C-025).
   - It worked on his studio PC, which had a recent VC++ redistributable.
3. **Months 1–5.**
   - About 30 project files used the plugin, several in Fixed-valves mode, with a keyswitch at the start of each item.
   - Reopened projects and playback started mid-song played the open-horn partials until the next keyswitch:
     - held valves are not part of the state (D-015);
     - `reset()` clears them (DECISIONS.md:34);
     - REAPER does not chase note-on keyswitches.
   - The owner thought he had mis-programmed the parts and re-recorded two of them.
4. **Month 5.**
   - He set up a second machine (laptop) and installed another vendor's plugin that loads an older app-local
     `msvcp140.dll` into REAPER's process.
   - The binary built with v14.5x then resolved its STL imports against the older DLL that was already loaded. JUCE's
     `WaitableEvent`/`Thread` use `std::mutex`, which is the known post-VS-17.10 `std::mutex` ABI hazard. REAPER
     crashed during plugin scan.
   - On the studio PC, a later Windows/REAPER update order reproduced the crash when opening any project containing the
     plugin. REAPER's crash recovery reopened the projects with FX offline, and two days of unsaved edits were lost.
5. **Month 6: attempted fix.**
   - The owner pushed a one-line change to `ci.yml`, the first push in 4 months.
   - `python` was red: `pip-audit` (DECISIONS.md:352) flagged a newly published CVE in `urllib3`/`requests`, which are
     transitive pins in `tools/requirements.lock`.
   - `realism` was red: the `tinysol-…` cache had been evicted after 7 days without access, and the Zenodo download
     was throttled.
   - `plugin` on `windows-2025` was red: the image had moved to a newer VS 2026 minor version that emits a new
     warning-as-error / breaking diagnostic in JUCE 9.0.3.
   - No scheduled run had warned him earlier. The plan's decision rules (DR-SECURITY, DR-ZENODO, DR-PERF) are addressed
     to the implementing agent, which no longer exists.
6. **Discovery.** The owner reported "plugin crashes REAPER, CI broken, can't rebuild" and could not tell which of the
   four failures to fix first. The project was abandoned for 3 weeks.

**Impact**
- **Host crash on project load:** unsaved work lost (Critical in effect, though caused by an environment interaction).
- **Silently wrong notes in reopened Fixed-valves projects:** user re-work.
- **Maintenance:** CI unusable for a solo maintainer without agent support.
- **Unverified gate:** the G-005 sign-off had been given on offline renders that never went through the plugin
  processor.

**How it was discovered:** by the end user in production use, not by any test or gate. pluginval (T-027) loads the
plugin in its own process with the runner's up-to-date runtime, and it does not exercise REAPER's state, chase or
reload behaviour.

**Root causes (detailed in §3):** R-010 (dynamic MSVC runtime), R-011 (no real-host acceptance and no deliverable
binary), R-014 (held valves not persisted or chased), R-019 (CI decays without a maintainer: blocking `pip-audit`,
cache eviction, image drift, no canary run).

---
## 2. Lenses

### 2.1 Technical correctness
- **Windows runtime linkage (R-010).**
  - Root `CMakeLists.txt:1-8` and `plugin/CMakeLists.txt:5-17` leave the MSVC runtime at CMake's default `/MD`. The
    VST3 built on `windows-2025` therefore imports `MSVCP140.dll`/`VCRUNTIME140*.dll` from the VS 2026 toolset (C-025).
  - Microsoft guarantees compatibility only when the loaded redistributable is at least as new as the toolset. An older
    copy already mapped into the host process (an app-local DLL loaded by another plugin) is a known crash source since
    the VS 17.10 `std::mutex` change.
  - No test or decision covers this. pluginval runs on the runner, which has the matching runtime, so it cannot catch
    it.
- **prepare() racing the GUI timer (R-015).**
  - The editor's 60 Hz timer calls `currentUiState()` → `voice.uiState()` on the message thread (D-013).
  - `TrumpetVoice::prepare()` "Allocates … Resets all state" (`core/include/tpt/TrumpetVoice.h`). The obvious
    implementation, `impl_ = std::make_unique<Impl>(…)`, frees the atomics that the GUI thread is reading. REAPER calls
    `prepareToPlay` again on a sample-rate or device change while the editor is open.
  - No test runs `prepare()` concurrently with `uiState()`, and there is no TSan job.
- **setStateInformation (R-016).**
  - D-015 does not say how the host buffer becomes a `string_view`.
  - `T-026 state round trip and garbage` (`tests/plugin/test_plugin.cpp`) passes `sizeof(junk)`, which includes a NUL
    terminator. A `std::string_view(static_cast<const char*>(data))` (strlen) implementation passes every frozen test
    but reads past a non-terminated host blob.
- **Fixed-valves state (R-014).**
  - The held valve combination is not part of `tpt-state-1`: D-015 stores only the 11 `VoiceParameters`.
  - It is reset by `reset()` (DECISIONS.md:34), and it is set only by note-on keyswitches, which hosts do not chase.
  - After a project reload, or playback started after the keyswitch, Fixed-valves parts silently play other partials.
    The UI shows the open horn, so a careful user can notice it.
- **Sample-rate clamp (R-022).** PLAN.md:374 clamps the rate passed to `voice.prepare` to [22050, 192000] while the
  host runs at its own rate. At 384 kHz (or 16 kHz) the instrument plays an octave or more off pitch, with only a log
  line.
- **Cross-platform floating point (R-021).**
  - `tpt_calibrate` writes fscale on Linux/GCC (x86-64: no FMA).
  - AppleClang on `macos-15` arm64 contracts `a*b+c` to FMA by default (`-ffp-contract=on`), and MSVC's libm `sin`/`cos`
    differ by an ulp.
  - The lip oscillator is chaotic near register boundaries, so T-012/T-013 may differ by OS. DR-REGIME/DR-CAL assume one
    platform: a macOS-only failure cannot be fixed by re-running `tpt_calibrate` on Linux.
- **Checked and found sound:**
  - D-006 oversampling choice and latency rounding;
  - D-004 validation limits against the T-006 cases;
  - D-009 hysteresis examples;
  - the D-012 limiter bound |y| < 1;
  - `embeddedResonatorJson()` via `CMAKE_CONFIGURE_DEPENDS` (DECISIONS.md:95-96).

### 2.2 Dependency and supply-chain drift
- **Pins hold.**
  - FetchContent pins are 40-hex (T-031), actions are SHA-pinned (D-020), and the pluginval zip is SHA-256 checked
    (`tests/scripts/run_pluginval.sh`).
  - JUCE 9.0.3 is pinned, while 8.x/9.x keep moving (C-001). That is fine for reproducibility, but security fixes in
    JUCE are not tracked by any step.
- **Drift that will happen within 6 months:**
  - `windows-2025` is a moving label: its VS 2026 minor version and Windows SDK change monthly (C-025). C-025's notes
    say "Whether JUCE 9.0.3 officially supports VS2026 was not checked".
  - `macos-15` loses support when macOS 15 images are deprecated. C-024 notes that macOS 14 is fully unsupported by
    2026-11-02; macOS 15 follows the same cadence.
  - The apt package set on `ubuntu-24.04` drifts.
  - Nothing runs on a schedule, so the owner learns about drift only on his next push (R-019).
- **Python lock (R-020).**
  - `tools/requirements.lock` pins versions but not hashes (no `--require-hashes`). `get-pip.py` is fetched unpinned
    and piped to Python (ENVIRONMENT.md:49).
  - The lock includes `requests`, `urllib3`, `certifi` and `cryptography`-adjacent packages that receive CVEs often. With
    `pip-audit` blocking in the `python` job (DECISIONS.md:352), a red CI with no code change is near-certain over
    6 months (R-019).
- **Zenodo / TinySOL.**
  - Record 3685367 is persistent, and the md5 is pinned (C-072).
  - `actions/cache` entries are evicted after 7 days without access, so a hobby cadence re-downloads the archive on
    almost every run (R-019).

### 2.3 Invalid research assumptions
Every claim whose confidence is `single-source` or `inferred` was reviewed:

| Claim | Confidence | Load-bearing | Finding |
|---|---|---|---|
| C-048 | inferred | no | The nonlinear-propagation algorithm was derived from eq. (2), not from Vergez & Rodet or Msallam (no full text). D-012 rests on it. It was spiked only at L_nl ∈ {0, 0.85} m (`timbre_spike.py:62`), while the brightness control maps it to 0–1.7 m (DECISIONS.md:207). This is the control-range gap in R-013. |
| C-049 | inferred | no | Superseded in practice: loops are tuned by bisection (D-004) and checked by T-007 (±5 c). No risk. |
| C-051 | single-source | no | The radiation high-pass shape is not used directly: D-004 fits the radiation sections to the TMM's jωH/Z_rad. No risk. |
| C-055 | inferred | no | C vs B♭ timbre comparability. Already R-003 (accepted); G-005 is the arbiter. No new risk. |
| C-063 | single-source | no | Arban matrix: alternates are corroborated by C-062, and the human checks them at G-004. No risk. |

Load-bearing claims derived from short spikes:
- **C-091 (sustain over 4 s), C-094 (≥ 4.9 s at soft pressure), C-093 (register jumps held 3 s).** T-012 renders only
  2 s, and T-023's longest single note is about 0.1 s (`tests/core/test_voice_robustness.cpp:91-110`). Musical use
  holds notes for 10–30 s, so this was re-checked.
  - **Independent 20 s re-run:** spike model, all 57 table rows × pp/mf/ff, pp schedule 1.3 → 0.8 × threshold over
    150 ms, D-023 floors.
  - **Result:** 169/171 runs keep the intended partial at 4 s and at 20 s, with RMS at 20 s ≥ 50 % of RMS at 4 s.
  - **The 2 flagged runs are the already-dropped alternate written 80** (1-2-3 p9; it locks to p10 at mf/ff; C-081).
    The fixture removed it.
  - **Conclusion:** the short-spike assumption holds for the spike model.
  - **Residual:** the shipped voice adds breath noise on the flow (D-012) and uses the final complex-modal table (C-099).
    No frozen test holds a note longer than 2 s, so a cheap non-frozen 20 s check is proposed under R-023.
- **C-089 (onset grid) and C-085/C-094 (timbre MADs).** Thin pp margins are already R-001 and R-006.
- **Provenance of C-091–C-099.**
  - These claims are labelled `verified`, Tier 1, but the source is an agent's report (`round-3-adversarial.md`).
  - The re-run scripts are archived (`research/spikes/adversarial/*.py`), but their outputs are not committed. Only
    `floors_cap_check_result.txt` and `dyn_spike_result.txt` back part of them.
  - The scripts hard-code `/home/claude/projects/...` paths (`adversarial/sustain.py:2-6`).
  - So the specific counts (125/133, ≥ 4.9 s) cannot be regenerated from committed results (R-023, Low; my 20 s re-run
    agrees in substance).
- **C-015 and C-029** (pluginval flag, xvfb) were inferences, now verified in CI by plan-verify run 37748754510. No risk.

### 2.4 Implementer misinterpretation
- **G-002 vs CI artifacts (R-011).**
  - GATES.md:33 requires G-002 to "upload binaries anywhere", yet D-020 uploads `tpt_render` as an artifact
    (DECISIONS.md:344).
  - An implementer either treats the `tpt_render` artifact as a violation or refuses to upload the VST3 the owner
    needs. Either way it must guess.
- **`prepare()` re-creating `Impl`** (R-015) and **`string_view` from a host buffer without its size** (R-016); see
  §2.1.
- **S-017 "Outputs: fixes as needed"** (PLAN.md:495).
  - DR-PERF step (3) (12 modes) and any S-017 voice fix change the sound after G-005 was signed.
  - No rule says to re-run T-029 or re-issue G-005 (R-017).
- **S-016 round logs and S-018 `REPORT.md`/README.** The plan says "Write" (PLAN.md:463, 512-513) and does not say
  "generate from the committed `results.json`", so the numbers in them can be hand-transcribed (R-018).
- **Checked and found unambiguous:**
  - keyswitch routing (D-003, D-014);
  - the CC16 precedence rule;
  - legato vs re-attack;
  - the gate-evidence trigger (DECISIONS.md:372-377);
  - `--keep-calibration`.

### 2.5 Integrity (Rule 9)
- **Results are overwritten.** `ci-results` is rewritten on every run with `git push --force` (DECISIONS.md:360, 369-371).
  - The `results.json` behind any number quoted in `REPORT.md`, `logs/S-016-round-<k>.md` or `GATE-G-005.md` is
    destroyed by the next push.
  - Reported numbers would no longer trace to a committed result, and a favourable run could be quoted without
    disclosure while later, worse runs overwrite the evidence (R-018).
  - The G-005 bundle's `results.json` on `evidence/G-005` survives, but only for the bundle commit.
- **Planning integrity.**
  - Frozen `EXPECTED:` literals are checked (`research/spikes/expected_values.py --check`).
  - `onset_ramp100_ms`/`onset_ramp200_ms` are copied from `t028_values.py`, which asserts them but is not run by
    `verify_freeze.sh`. This is acceptable because both files are frozen by hash (manifest line for
    `expected_values.py`).
  - C-091–C-099 provenance: see §2.3 (R-023).
- **No fabricated data found:**
  - the T-029 docstring figures match C-073/C-085/C-089/C-094;
  - the TMM-skip statement in S-005 ("fails without the skip") is true (§2.10).

### 2.6 Past lessons (every row of `research/PRIOR_KNOWLEDGE.md`)
| # | Lesson (severity) | Prevention rule applied by | Verdict |
|---|---|---|---|
| 0 | L-20261008T151500Z reasoned expected value (High) | `t028_values.py`, `expected_values.py` names it; S-004 | Applied. The asserting script is not in the freeze check, but both are frozen; no risk. |
| 1 | L-20261008T010500Z circular FFT (High) | C-089 re-run; T-028 "slower attack measures later"; S-004/S-005/S-015 cite it | Applied. |
| 2 | L-20261007T150300Z hand-typed constant (High) | `expected_values.py --check` in T-032; S-003 generated table | Applied. |
| 3 | L-20261007T150400Z threshold vs reference spread (High) | `ref_spread.py`, T-029 docstring | Applied (margins thin: R-001). |
| 4 | L-20261007T150500Z linear-bore ceiling (High) | `timbre_spike.py`, `dyn_spike.py`, D-012 | Applied. |
| 5 | L-20261007T150600Z control mappings unverified (High) | Overblow, dynamics, attack spiked; lip stiffness carried as R-004 | **Not fully applied.** Brightness (L_nl 0–1.7 m, DECISIONS.md:207) spiked only at 0 and 0.85 m. Breath noise (DECISIONS.md:212-215) never spiked: the 5 % flow modulation at breathNoise = 1 acts on pp notes sustained by hysteresis at 0.8 × threshold. Tuning 415–466 Hz (D-011 A4/440 factor, ±6 % on every pole and f_l, thresholds unchanged) never spiked. Vibrato depth/rate never spiked with the lips. T-023 checks only finiteness and bounds at the extremes (`test_voice_robustness.cpp:16-34`), not lock, sustain or timbre. The default equals the verified point; the range does not. → **R-013, High** (lesson severity). |
| 6 | L-20261007T150700Z regime per note (High) | `regime_map.py`, C-091, T-012 sustain gate | Applied; confirmed by my 20 s re-run (§2.3). |
| 7 | L-20261007T150000Z sandbox no pip (Medium) | ENVIRONMENT no-sudo variant; S-000 | Applied. |
| 8 | L-20261007T150100Z credentials (Medium) | HANDOFF, S-000, S-KNOW secret scan | Applied for the implementer. A-020's pasted PATs are outside it: R-012. |
| 9 | L-20261007T150800Z host accessors (Medium) | D-013 accessors; `releaseOverblowControl()` additive | Applied. The missing piece is persistence of held valves, not an accessor (R-014). |
| 10 | L-20261007T150200Z restart (Low) | HANDOFF halt protocol | Applied. |
| 11 | L-20261007T150900Z sha256sum backslash (Low) | `run_pluginval.sh` hashes via stdin | Applied. |
| 12 | L-20261007T151000Z evidence off impl branch (Low) | D-019 orphan branches; `.gitignore` | Applied. |

### 2.7 Scale and performance
- **Load estimate.** At 48 kHz, os = 2 → 96 kHz internal:
  - 14 complex one-pole modes;
  - a closed-form flow solve;
  - the time-warp ring buffer;
  - ≤ 6 biquads;
  - 63-tap half-band decimation.

  That is a few tens of Mflop/s, an RTF of about 0.01–0.03 against the 0.05 limit (T-024b). The margin is about 2×,
  and DR-PERF exists. A noisy macOS runner can make T-024b flaky, but no register entry is needed.
- **Session uptime.** A REAPER session left open for days was considered. D-006 requires double precision inside the
  voice, and the idle rule stops processing. No test runs longer than 30 s, but nothing found would degrade with uptime
  if D-006 is followed. No entry.
- **Long notes:** covered by the 20 s re-run (§2.3).

### 2.8 Security (threat model A-015)
- **Host state blobs.**
  - 64 KiB limit, `parse(…, nullptr, false)`, type checks, and nesting depth 2000 (T-011) are good.
  - The gap is the buffer-to-string conversion (R-016).
- **Embedded table.**
  - It is compiled in and validated by T-007 in CI on every OS.
  - A table that fails validation would throw in the processor constructor. JUCE's factory does not catch that, so the
    host crashes at instantiation. T-007 makes this unlikely, so no entry.
- **CI supply chain (R-020).**
  - `realism` and `gate-evidence` run with `contents: write` and execute PR-controlled Python/C++ plus unhashed PyPI
    dependencies.
  - Without branch protection, `GITHUB_TOKEN` with `contents: write` can push to `main` (bypassing G-002) if a
    dependency is compromised.
  - On fork PRs the token is read-only, so the push steps just fail (R-024).
- **Tokens (R-012).**
  - A-020 (ASSUMPTIONS.md:62) records two PATs pasted in chat:
    - the first "should be revoked by the human";
    - the second still lives in the planning container's keyring.
  - Nothing verifies or schedules their revocation. No gate, HANDOFF item or S-018 report line asks for it.
  - A live token with Contents and Workflows write on `qualitycoding/trumpet-vst`, sitting in a chat transcript, is a
    security exposure.

### 2.9 Operational realities (R-019, R-011, R-024)
- **CI decays without a maintainer.** After handoff, the decision rules are addressed to an agent that no longer
  exists. Over 6 months, expect:
  - `pip-audit` red on a new CVE;
  - TinySOL cache eviction, then a Zenodo throttle or outage turning `realism` red;
  - runner image minor updates on `windows-2025` and `macos-15`;
  - the macOS 15 label deprecation.

  There is no `schedule:` canary, no toolchain-version log, and no "maintenance runbook" for the owner. (For a public
  repository, GitHub disables scheduled runs after 60 days of inactivity, so even a canary needs care.)
- **No `concurrency:` group.**
  - `on: [push, pull_request]` runs `realism` twice for a PR head, and both force-push `ci-results`.
  - Two quick pushes race, and the older commit's results can win (R-024).
- **Host behaviour pluginval does not cover.** pluginval loads the plugin in isolation on a runner. It does not cover:
  - REAPER's anticipative FX and multi-threaded processing;
  - project save/reload and undo-state churn;
  - note chase on playback start;
  - a sample-rate change with the editor open (R-015);
  - the runtime DLL environment of a real machine (R-010);
  - the owner's real controllers (CC2/CC11 breath).

  A-014 names Windows 11 + REAPER as the human test setup, yet no gate asks the human to load the plugin there (R-011).

### 2.10 Re-check of the four cold-read pass-5 fixes (R-009)
1. **TMM short-frustum skip (S-005, `tools/resonator/tmm.py` docstring).**
   - Verified independently. The staircase in `test_t030_cone_vs_staircase` contains 1999 frusta of length ≈ 1e-9 m:
     - with the skip (L < 1e-8 → identity) the first six peaks match the cone within 0.09 % (rtol 2e-3, so a 2.2×
       margin): 148.62/301.39/459.12/620.60/784.56/950.13 Hz vs 148.65/301.77/459.70/621.30/785.34/950.98 Hz;
     - without the skip the peak list collapses (148.0, 148.3, 148.7, 149.4, 300.7, …) and the test fails, exactly as
       stated.
   - **Closed.**
2. **`realism` step guards (D-020).**
   - The `!cancelled() && steps.chk.outputs.present == '1'` guards are correct.
   - Ordering is correct: `git checkout --orphan ci-results; git rm -rf .` must run after pytest, and DECISIONS.md:357
     orders summarize → T-029 → push.
   - Residual: no `concurrency:` group and duplicate push/PR runs (R-024, Low). **Closed with residual.**
3. **S-001 perf red run.** It runs in Release, the stubs throw, and the red expectation is stated. **Closed.**
4. **S-017 dependency.**
   - It now covers `proceed`, `proceed-with-rescope` and `override-intonation`; `iterate` loops back.
   - Residual: S-017 itself can change the sound after G-005 (R-017, Medium). **Closed with residual.**

R-009 can move to `closed`. Its residuals are carried as R-017 and R-024.

---
## 3. Root cause analysis

Severity follows the rubric in the request. Likelihood is L, M or H.

### R-010 — Windows VST3 links the MSVC runtime dynamically (VS 2026 toolset)
- **Lens:** technical / operational. **Severity: High** (the plugin fails to load, or crashes the host, on machines
  whose loaded `msvcp140.dll` is older than v14.5x; the workaround is undocumented). **Likelihood: M.**
- **Root cause:**
  - `CMakeLists.txt:1-8` does not set `CMAKE_MSVC_RUNTIME_LIBRARY`, so the default is `/MD`.
  - The `windows-2025` runner uses VS 2026 (C-025).
  - No check inspects the binary's imports. pluginval runs where the runtime matches.
- **Traces to:** C-025, D-020 (`plugin` job), D-022, T-027.
- **Mitigation:**
  - New decision D-024: in the root `CMakeLists.txt`, before any `FetchContent_MakeAvailable`, add
    `set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")` (CMP0091 is NEW under
    `cmake_minimum_required(3.22)`).
  - New CI check in the Windows `plugin` job (workflow-level, not frozen): run `dumpbin /dependents` on
    `Trumpet.vst3/Contents/x86_64-win/Trumpet.vst3` and fail with `::error::` if `MSVCP140`, `VCRUNTIME140` or
    `VCRUNTIME140_1` appear.
  - README "Building": state the toolset and that the runtime is statically linked.

### R-011 — No real-host acceptance; no installable binary for the owner
- **Lens:** operational / implementer misinterpretation. **Severity: High** (the implementer is forced to guess
  whether a CI artifact counts as "upload binaries anywhere" under G-002, which conflicts with D-020's `tpt_render`
  artifact; the SC-1 user outcome is never verified). **Likelihood: H.**
- **Root cause:**
  - A-014 names Windows 11 + REAPER as the human test setup, but G-004/G-005 evidence is screenshots and offline
    `tpt_render` WAVs that bypass `PluginProcessor`.
  - D-020 uploads no plugin artifact (DECISIONS.md:344).
  - GATES.md:33 forbids uploading binaries "anywhere" without G-002.
  - S-018 records "a real DAW host test" as not verified (PLAN.md:513) and accepts that.
- **Traces to:** A-014, G-002, G-004, G-005, D-020, S-014, S-016, S-018, SC-1.
- **Mitigation:**
  - Amend G-002's trigger: workflow artifacts on the private repository with ≤ 14-day retention are not a release.
  - Add an `upload-artifact` step for `Trumpet.vst3` (Windows, macOS, Linux) to the `plugin` job.
  - Add a human gate G-006 "host check" after S-017: the owner installs the Windows artifact into REAPER and runs a
    fixed checklist:
    - plugin scan;
    - insert on a MIDI track;
    - play the G-005 demo MIDI files;
    - automate Overblow;
    - CC2 breath;
    - save, close and reopen the project (state restored, Fixed-valves keyswitch behaviour);
    - change the sample rate with the editor open;
    - render.
  - Allowed responses: `proceed` / `fix: <items>`. The outcome goes into `REPORT.md`.

### R-012 — Pasted PATs (A-020) are never confirmed revoked
- **Lens:** security. **Severity: Critical** (security exposure: tokens with repository write and workflow scope in a
  chat transcript and in a planning container keyring). **Likelihood: M.**
- **Root cause:**
  - A-020 (ASSUMPTIONS.md:62) leaves revocation to "the human" with no gate, reminder or check.
  - The second token is never scheduled for revocation at all.
- **Traces to:** A-020, L-20261007T150100Z, HANDOFF credentials section.
- **Mitigation:**
  - Add an "Open human actions" block to `HANDOFF.md` and a mandatory line in `GATE-G-004.md` and `REPORT.md`: "Revoke
    PAT #1 and the planning PAT #2 (GitHub → Settings → Developer settings → tokens); reply `revoked`".
  - S-000: the implementer confirms its own credential is distinct from both, by name and expiry via `gh auth status`,
    without printing the token.
  - Track this as a G-002-style precondition: no gate after G-004 is processed until `revoked` is recorded.

### R-013 — Control→model mappings not exercised over their full range (recurrence of L-20261007T150600Z)
- **Lens:** past lessons / technical. **Severity: High** (the lesson's severity: a recorded mistake is allowed to
  recur). **Likelihood: M.**
- **Root cause:**
  - Brightness maps L_nl over 0–1.7 m (DECISIONS.md:207) but was spiked only at 0 and 0.85 m (`timbre_spike.py:62`).
  - The breath-noise flow modulation (DECISIONS.md:212-215) and tuning 415–466 Hz (DECISIONS.md:189) were never spiked
    against regime lock and sustain.
  - The vibrato/bend interaction with the lips was not spiked.
  - T-023 checks only finiteness and bounds at the extremes (`tests/core/test_voice_robustness.cpp:16-34`).
  - The likely failure is harsh shock artefacts or aliasing at brightness 1 and ff, or pp notes dropping out at
    breathNoise 1 or tuning 466.
- **Traces to:** D-011, D-012, C-048, C-085, T-012, T-022, T-023, lesson #5.
- **Mitigation:**
  - Add a non-frozen sweep to S-011, `tools/render/range_check.cpp` → `logs/S-011-range-check.md`:
    - the 36 standard notes;
    - pp/mf/ff;
    - brightness {0, 1}, breathNoise {0, 1}, tuning {415, 466}, vibrato depth 1 at 8 Hz, bend ±2;
    - criteria: T-012's sustain and partial criteria; at brightness 1, energy above 16 kHz less than −40 dB re the total
      (aliasing).
  - Add DR-RANGE: on failure, narrow the mapping in `VoiceTuning.h` (logged), never the parameter range in
    `Parameters.h`. Add the evidence to G-005 (demo 09, brightness sweep).
  - Propose a TEST_CHALLENGE amendment adding T-012b (sustain at the extremes).

### R-014 — Fixed-valves held valves not persisted and not chased
- **Lens:** technical / operational. **Severity: Medium** (wrong partials after a project reload or a mid-song start;
  the workaround is to re-send the keyswitch, which must be documented). **Likelihood: M** for Fixed-valves users.
- **Root cause:**
  - D-015 stores only `VoiceParameters`.
  - Held valves are reset by `reset()` (DECISIONS.md:34) and set only by note-on keyswitches, which hosts do not chase.
- **Traces to:** D-003, D-015, T-011, T-026.
- **Mitigation:**
  - Add an optional `"heldValves"` field to `tpt-state-1`. It is additive, and readers ignore it when absent (T-011
    still passes because unknown or missing fields keep defaults).
  - The processor restores it after `setStateInformation` via `keyswitch()`.
  - `reset()` called by the processor on `prepareToPlay` must not clear it (D-003 wording: "reset by `reset()` unless
    restored from state").
  - Document in the README: "place a keyswitch at the start of every Fixed-valves item".

### R-015 — `prepare()` vs GUI-thread `uiState()` race
- **Lens:** implementer misinterpretation / operational. **Severity: Medium** (a host crash on a sample-rate or device
  change with the editor open; the workaround is to close the editor first). **Likelihood: L–M.**
- **Root cause:**
  - The header says `prepare()` "Allocates … Resets all state".
  - Nothing forbids replacing `impl_`, while D-013 has the message thread reading the atomics at 60 Hz.
- **Traces to:** D-006, D-013, S-008, S-013, T-026.
- **Mitigation:**
  - D-013 rule: "the UI atomics live in storage that `prepare()` never frees; `prepare()` reallocates buffers inside
    `Impl`, never `impl_` itself".
  - Add a non-frozen stress test, `tests/stress/test_prepare_ui_race.cpp`: one thread calls `prepare(44100/48000/96000)`
    in a loop while another polls `uiState()`. Run it under `-fsanitize=thread` in the Linux `core` job.

### R-016 — Host state blob converted without its size
- **Lens:** security / implementer misinterpretation. **Severity: Medium** (an out-of-bounds read on crafted or
  non-terminated host data; host crash; A-015 rates the threat Low). **Likelihood: L.**
- **Root cause:**
  - D-015 does not specify the conversion.
  - The frozen garbage case passes a NUL-terminated buffer (`tests/plugin/test_plugin.cpp`, `sizeof(junk)`).
- **Traces to:** A-015, D-015, S-013, T-026.
- **Mitigation:**
  - D-015: `if (data == nullptr || sizeInBytes <= 0) return;` then
    `tpt::deserializeState({static_cast<const char*>(data), static_cast<size_t>(sizeInBytes)})`.
  - Amendment: add a T-026 case with a buffer that is exactly `"{\"format\":\"tpt-state-1\""` with no terminator,
    followed by a guard page or ASan in the Linux plugin job.

### R-017 — G-005 sign-off can go stale after S-017
- **Lens:** implementer misinterpretation / integrity. **Severity: Medium** (the shipped sound differs from what the
  human approved; the workaround is to re-listen). **Likelihood: L–M.**
- **Root cause:**
  - S-017 "Outputs: fixes as needed" (PLAN.md:495).
  - DR-PERF step (3) reduces modes. S-017 can also fix voice code for pluginval.
  - No rule ties G-005 to the commit of its bundle.
- **Traces to:** S-017, DR-PERF, DR-PLUGINVAL, G-005.
- **Mitigation:**
  - Record the G-005 bundle's source sha in `GATE-G-005.md`.
  - S-017/S-018 done-when: if `git diff <bundle sha>..HEAD` touches `core/src/TrumpetVoice.cpp`,
    `core/src/VoiceTuning.h`, `core/src/LipModel.cpp` or `data/trumpet_resonators.json`, re-run T-029 and the demos,
    and re-issue G-005 with a per-demo RMS-difference table.

### R-018 — Reported realism numbers are not traceable to committed results
- **Lens:** integrity (Rule 9). **Severity: Medium** (report numbers can be hand-transcribed or selectively quoted,
  and the underlying `results.json` is destroyed by the next force-push; this is not silent corruption of the plugin
  itself). **Likelihood: M.**
- **Root cause:**
  - `ci-results` is overwritten with `git push --force` (DECISIONS.md:360, 369).
  - S-016 round logs and `REPORT.md`/README are "written" (PLAN.md:463, 512-513), not generated.
- **Traces to:** D-020, S-016, S-018, Rule 9.
- **Mitigation:**
  - Commit each round's `results.json` (small JSON) and `SUMMARY.md` to `logs/S-016-round-<k>/` on the implementation
    branch.
  - Make `ci-results` append-only: a directory per source sha, with no `--force` beyond the orphan creation.
  - `REPORT.md`'s metric tables are produced by `tools/realism/summarize.py` from a named committed file, and REPORT
    cites that file and its sha.
  - S-018 adds a check that each number in REPORT's realism table appears in the cited `SUMMARY.md`.

### R-019 — CI decays without the implementing agent (solo maintainer)
- **Lens:** operational / dependency drift. **Severity: Medium** (CI turns red with no code change; the workarounds
  exist, but only as agent decision rules). **Likelihood: H** over 6 months.
- **Root cause:**
  - `pip-audit` blocks the `python` job (DECISIONS.md:352) on transitive pins that get CVEs often
    (`urllib3`, `requests`, `certifi`).
  - The TinySOL cache is evicted after 7 days idle, and Zenodo becomes a live dependency of the `realism` job.
  - The `windows-2025`/`macos-15` images drift (C-024, C-025).
  - There is no scheduled canary and no maintainer runbook.
- **Traces to:** A-017, D-020, DR-SECURITY, DR-ZENODO, C-023–C-025.
- **Mitigation:**
  - Move `pip-audit` to its own job that is non-blocking on push (`continue-on-error: true` plus a `::warning::`) and
    blocking only on a monthly `schedule:` run, because Python is test-only (DR-SECURITY already says so).
  - Add `schedule: cron "0 6 1 * *"` for the whole workflow. Each job logs `cl`/`cmake`/Xcode/apt versions to its step
    summary.
  - Make `realism` informational on push when the TinySOL download fails (`::warning::`, job green), and red only when
    the data is present and T-029 fails.
  - Write `MAINTENANCE.md` in S-018: what each job does, the DR rules restated for a human, how to bump a pin.

### R-020 — Write-token CI jobs execute unhashed third-party Python
- **Lens:** security (CI supply chain). **Severity: Medium** (a compromised PyPI release could push to any branch,
  including `main`, if branch protection is absent; it ships nothing to users). **Likelihood: L.**
- **Root cause:**
  - `tools/requirements.lock` has no hashes.
  - `get-pip.py` is unpinned (ENVIRONMENT.md:49).
  - `realism`/`gate-evidence` hold `contents: write` (DECISIONS.md:362) while running Python, and `main` protection is
    not required by the plan.
- **Traces to:** D-020, A-016, G-002, T-031.
- **Mitigation:**
  - Regenerate the lock with `--generate-hashes` and install with `--require-hashes`.
  - Split each push into a final job with `contents: write` that only downloads the results artifact and pushes (no
    Python, no build).
  - Ask the human at G-004 to enable branch protection on `main` (no direct pushes; G-002 enforced by GitHub).

### R-021 — Calibration on Linux, FP behaviour differs on macOS arm64 and Windows
- **Lens:** technical. **Severity: Low** (pitch differences are expected to be sub-cent; regime flips only near
  boundaries, and T-012/T-013 in the 3-OS `core` job would show them). **Likelihood: L.**
- **Root cause:**
  - AppleClang's default `-ffp-contract=on` gives FMA on arm64, and libm differs across platforms.
  - DR-REGIME/DR-CAL do not address OS-specific failures.
- **Traces to:** D-006, D-011, DR-CAL, DR-REGIME, T-012, T-013, T-023.
- **Mitigation:**
  - `target_compile_options(tpt_core PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-ffp-contract=off>)`.
  - New DR rule: a failure on one OS only → first compare `tpt_calibrate` output across OSes (run it in CI on all 3 and
    upload the JSON). Never commit an OS-specific table.

### R-022 — Host sample rates outside [22.05, 192] kHz play at the wrong pitch
- **Lens:** technical. **Severity: Low** (rare configurations; audible). **Likelihood: L.**
- **Root cause:** PLAN.md:374 clamps the rate passed to `voice.prepare` while the host runs at its own rate.
- **Traces to:** S-013, D-006.
- **Mitigation:** outside the range, output silence, set a `sampleRateUnsupported` flag shown by the editor caption,
  and log once. Document the range in the README.

### R-023 — Short-spike claims rest on uncommitted re-run outputs; no long-hold check of the shipped voice
- **Lens:** research assumptions / integrity. **Severity: Low** (my 20 s re-run of the spike model reproduces the
  sustain claims: 169/171, and the 2 flagged runs are the dropped alternate written 80). **Likelihood: L.**
- **Root cause:**
  - The outputs of C-091/C-093/C-094 live only in the R5 report.
  - `research/spikes/adversarial/*.py` hard-code absolute paths.
  - No frozen test holds a note longer than 2 s.
- **Traces to:** C-091, C-093, C-094, T-012, T-023.
- **Mitigation:**
  - Commit the re-run outputs, or mark the counts `corroborated` instead of `verified`.
  - Add to S-009 a non-frozen `tpt_render` check: every standard note at pp/mf/ff held 20 s, partial and RMS ratio
    (≥ 0.5 between 4 s and 20 s) logged in `logs/S-009-long-hold.md`. Apply DR-REGIME on failure.

### R-024 — CI concurrency and event duplication in `realism`
- **Lens:** operational. **Severity: Low** (stale `ci-results`; red jobs on fork PRs). **Likelihood: M.**
- **Root cause:**
  - `on: [push, pull_request]` with no `concurrency:` group.
  - Force-push to `ci-results` (DECISIONS.md:360).
  - Fork PRs have a read-only token.
- **Traces to:** D-020, R-009 (pass-5 item 2 residual).
- **Mitigation:**
  - `concurrency: {group: ci-${{ github.ref }}, cancel-in-progress: true}`.
  - Run the `realism` push step only when `github.event_name == 'push'`.

---

## 4. Proposed register entries

| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-010 | Windows VST3 links the MSVC runtime dynamically (VS 2026 toolset) → load failure or host crash with an older loaded `msvcp140.dll` | technical / operational | High | M | No `CMAKE_MSVC_RUNTIME_LIBRARY`; nothing inspects the binary's imports | C-025, D-020, D-022, T-027 | D-024 static CRT; CI `dumpbin /dependents` check; README note | open |
| R-011 | No real-host acceptance; no installable binary; G-002 "upload binaries anywhere" conflicts with D-020 artifacts | operational / misinterpretation | High | H | Gates use offline renders and screenshots only; no plugin artifact | A-014, G-002, G-004, G-005, D-020, S-018, SC-1 | Amend G-002 (CI artifacts are not a release); upload the VST3 artifact; new gate G-006 REAPER host checklist | open |
| R-012 | Pasted PATs (A-020) never confirmed revoked | security | Critical | M | Revocation left to the human with no gate or reminder | A-020, L-20261007T150100Z | HANDOFF "open human actions"; `revoked` confirmation required before G-005; S-000 confirms a distinct credential | open |
| R-013 | Brightness (0–1.7 m), breath noise, tuning 415–466 and vibrato mappings not exercised over their range (lesson #5 recurrence) | past lessons / technical | High | M | Spikes covered defaults only; T-023 checks only finite and bounded | D-011, D-012, C-048, T-012, T-022, T-023 | S-011 range sweep + DR-RANGE; G-005 demo 09; T-012b amendment | open |
| R-014 | Fixed-valves held valves not persisted or chased → wrong partials after reload or mid-song start | technical / operational | Medium | M | D-015 stores only `VoiceParameters`; `reset()` clears held valves | D-003, D-015, T-011, T-026 | Optional `heldValves` state field; `prepareToPlay` must not clear it; README keyswitch guidance | open |
| R-015 | `prepare()` replacing `Impl` races the 60 Hz GUI `uiState()` read → crash on a rate change with the editor open | misinterpretation / operational | Medium | L–M | Header and D-013 do not forbid freeing the UI atomics | D-006, D-013, S-008, S-013 | D-013 storage rule; TSan stress test in the Linux `core` job | open |
| R-016 | Host state blob converted without `sizeInBytes` → out-of-bounds read | security / misinterpretation | Medium | L | D-015 silent on conversion; frozen junk case is NUL-terminated | A-015, D-015, S-013, T-026 | Sized `string_view`; non-terminated-buffer case under ASan (amendment) | open |
| R-017 | G-005 sign-off goes stale when S-017 changes sound-affecting code | misinterpretation / integrity | Medium | L–M | "fixes as needed"; DR-PERF (3); no bundle-sha rule | S-017, DR-PERF, G-005 | Record bundle sha; re-run T-029 and demos and re-issue G-005 if voice/table files changed | open |
| R-018 | Realism numbers not traceable to committed results (force-pushed `ci-results`; hand-written logs and REPORT) | integrity | Medium | M | Overwrite-only results branch; "write" not "generate" | D-020, S-016, S-018, Rule 9 | Commit per-round `results.json`; append-only `ci-results`; REPORT tables generated by `summarize.py` and cross-checked | open |
| R-019 | CI decays without the agent: blocking `pip-audit`, cache eviction + Zenodo, runner image drift, no canary | operational / drift | Medium | H | Decision rules addressed only to the implementer; no schedule | A-017, D-020, DR-SECURITY, DR-ZENODO, C-023–C-025 | Non-blocking `pip-audit` on push; monthly schedule; tolerant `realism`; toolchain logging; `MAINTENANCE.md` | open |
| R-020 | Write-token CI jobs run unhashed PyPI dependencies; `main` unprotected | security | Medium | L | No `--require-hashes`; unpinned `get-pip`; push in the same job as Python | D-020, A-016, G-002, T-031 | Hashed lock; push-only job; branch protection on `main` | open |
| R-021 | Calibration on Linux; FMA/libm differences on macOS arm64 and Windows may flip borderline regimes | technical | Low | L | Default FP contraction; DR rules assume one OS | D-006, D-011, DR-CAL, T-012, T-013 | `-ffp-contract=off` for `tpt_core`; per-OS calibrate comparison rule | open |
| R-022 | Host rate outside [22.05, 192] kHz → wrong pitch | technical | Low | L | Rate clamp in S-013 | S-013, D-006 | Silence + UI flag outside the range; README | open |
| R-023 | Short-spike claims backed by uncommitted outputs; no long-hold check of the shipped voice | research / integrity | Low | L | R5 outputs not committed; T-012 holds only 2 s | C-091, C-093, C-094, T-012 | Commit outputs or downgrade; S-009 20 s long-hold log (spike re-run: 169/171 OK) | open |
| R-024 | `realism` concurrency races and fork-PR push failures | operational | Low | M | No `concurrency:` group; push on `pull_request` | D-020, R-009 | Concurrency group; push only on `push` events | open |

Disposition of existing entries: **R-009 → closed.** All four pass-5 fixes were re-verified (§2.10). The TMM skip was
confirmed by an independent run, and the residuals are carried as R-017 and R-024. R-001..R-008 stay unchanged; no
new evidence affects them. The 20 s re-run supports R-007/R-008 being Low.

COUNTS: Critical 1, High 3, Medium 7, Low 4
