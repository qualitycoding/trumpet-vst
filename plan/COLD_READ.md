# Cold-read gate log (protocol 3.6)

| Pass | Agents (fresh context) | Items | Result |
|---|---|---|---|
| 1 | Sonnet dry run (interrupted twice by rate limits; 50 items recorded before the final stop) + Haiku mechanical check | 50 dry-run items (7 High, 23 Medium, 20 Low); mechanical: 8/8 checks PASS | All 50 resolved in commit 196ec49 (see below) |
| 2 | New Sonnet dry run (report complete, 28 items, then rate-limited) + new Haiku mechanical check | 28 items (3 High, 8 Medium, 17 Low); mechanical: 0 failures | All 28 resolved (see below) |
| 3 | New Sonnet dry run + new Haiku mechanical check | 10 items (1 High, 4 Medium, 5 Low); mechanical: 0 failures | All 10 resolved (see below) |
| 4 | New Sonnet dry run (resumed once after a rate limit) + new Haiku mechanical check | 5 items (0 High, 0 Medium, 5 Low); mechanical: 0 failures (the agent misreported two counts; re-checked by the planner: 30 manifest files, 33 T-IDs) | All 5 resolved (see below) |
| 5 (last allowed) | New Sonnet dry run (ran frozen inputs against the spikes) | 4 items (0 High, 2 Medium, 2 Low) | All 4 resolved by amendment; no further pass allowed → R-009 (Medium), re-checked in pre-mortem round 1 |

## Pass 1 disposition (dry-run item → fix)
- **High**
  - **5:** accepted-red path after G-005 (`plan/GATES.md`; S-017/S-018 done-when).
  - **13/3:** T-014 moved to S-010, T-018 to S-011; S-010 implements the full `fscale_eff`.
  - **23:** T-028 expected onset recomputed with the reference metric (53/102 ms; `research/spikes/t028_values.py`; lesson `L-20261008T151500Z`, a recurrence of `L-20261007T150300Z`); re-frozen.
  - **24:** S-000 counts any HTTP status as reachable; host list extended.
  - **25:** `plan/PROTOCOL_EXTRACTS.md`.
  - **26:** CI job `gate-evidence` (D-020); S-014 snapshots from `UiState`.
  - **27:** `tpt_render --events`; demo event lists in G-005.
- **Medium**
  - **1:** A-016 branch name.
  - **2:** A-002 wording.
  - **4/33:** S-008 done-when names the one allowed failing assertion.
  - **6:** gate waiting protocol.
  - **7/48:** G-004 image list and names.
  - **8/30:** `ci-results` orphan branch; `realism` needs a build-only job; Windows `-C Release`.
  - **12/41:** latency rounding (`lround`).
  - **14/39:** D-011 bore-state rule; A-006 and G-005 Q4 wording.
  - **15:** release time (D-007).
  - **16:** single level meter (D-013).
  - **20:** D-015 keys, compact JSON, APVTS mapping.
  - **22/28/44:** D-004 number formatting; `tpt_calibrate` `%.10g`.
  - **29:** S-004 cross-check inputs.
  - **31:** Zc formula.
  - **32:** limiter after the decimator.
  - **34:** G-005 blind-pair recipe.
  - **35:** two-commit record convention.
  - **36:** `GH_TOKEN`, `gh auth setup-git`, git identity.
  - **37:** D-003 rule (2).
  - **38:** D-017 harmonic count and `--tinysol`.
- **Low**
  - **10:** S-RETRO without subagents.
  - **11:** `mkdir -p logs`.
  - **17:** invalid-input rule (D-014).
  - **18:** layout sizes.
  - **19:** hysteresis algorithm (D-009).
  - **21/49:** T-031 wording and WAV locations; notice versions from ENVIRONMENT.
  - **40:** S-001 red expectations with the plugin.
  - **42:** `tools/gen_fingering_table.py`.
  - **43:** `bore_fit` docstring.
  - **45:** R5 alias.
  - **46:** DR-REGIME (2b).
  - **47:** pitch-wheel formula.
  - **9, 50:** no action needed (as the reviewer noted).

Raw reports: `research/coldread/pass-1-dryrun.md`, `research/coldread/pass-1-mechanical.md`.

## Pass 2 disposition
- **High**
  - **1:** the venv lives outside the repository, symlinked as `.venv` (ENVIRONMENT, D-020, `.gitignore`).
  - **2:** `allNotesOff()` is a 4 ms panic (D-014); S-010 implements it.
  - **3:** `gate-evidence` is push-triggered by a commit-message marker (D-020, S-014, S-016).
- **Medium**
  - **4:** job list fixed (`build-linux`; six plus one jobs).
  - **5:** oversampling factors in {1, 2, 4}.
  - **6:** the flow law uses ρ = 1.2041.
  - **7:** the partial tracker works from the lip opening.
  - **8:** `tools/realism/summarize.py` (S-015).
  - **9:** re-calibrate after `VoiceTuning.h` changes; exclusive resources added.
  - **10:** the caption is drawn by `TrumpetView`.
  - **11:** `set_property(DIRECTORY … CMAKE_CONFIGURE_DEPENDS)`.
- **Low**
  - **12:** `--no-track` and `push -u`.
  - **13:** release-asset hosts.
  - **14:** scope recording.
  - **15:** tools row.
  - **16:** host-rate clamping and block chunking (S-013).
  - **17:** noise normalisation.
  - **18:** event units.
  - **19:** compare f0 and segments.
  - **20:** 13th lesson row; full lesson IDs.
  - **21:** `*.wav` in `.gitignore`.
  - **22:** store directory naming, TAXONOMY format, `make_index.py`.
  - **23:** c and ρ values.
  - **24:** velocity rule.
  - **25:** `ctest -LE 'perf|plugin'`.
  - **26:** cache path and forced push.
  - **27:** `noteOn` ignores 24–31.
  - **28:** `tpt_f0` rate.

## Pass 3 disposition
- **High**
  - **1:** step-level presence guard instead of a job-level `hashFiles` (D-020).
- **Medium**
  - **2:** `download-artifact` pin.
  - **3:** UI atomics are published by control calls (D-013).
  - **4:** plugin ctest label (`plugin/CMakeLists.txt`); S-001 plugin red step.
  - **5:** `stop` branches run the closing steps (GATES, S-RETRO).
- **Low**
  - **6:** `releaseOverblowControl()` additive member (D-014).
  - **7:** build before `tpt_calibrate`.
  - **8:** CI as an exclusive resource in S-014 and S-015.
  - **9:** `setParameters` follows `clamped()`.
  - **10:** reproducibility settings and the CI-regeneration fallback (S-005).

## Pass 4 disposition
- **1:** S-009 runs `tpt_operational_tests` and requires it fully green.
- **2:** the S-016 dependency covers a `correct-chart` amendment.
- **3:** `ctest -L perf` runs in its own `!cancelled()` step.
- **4:** the `realism` job runs `compare_tinysol` explicitly before pytest.
- **5:** TinySOL extraction marker for idempotency and cache hits.

## Pass 5 disposition
- **1:** short-frustum skip in the TMM port (S-005, `tmm.py` docstring).
- **2:** `!cancelled()` guards on the `realism` summarize, artifact and push steps (D-020).
- **3:** perf red run in S-001.
- **4:** the S-017 dependency covers `override-intonation`.

**Cold-read gate result:** 97 items over 5 passes, all resolved. The mechanical checks were clean in passes 2–4. The final
pass's fixes are carried as R-009.

## Re-run after pre-mortem round 1 hardening (pass 6, protocol 4.4)
A fresh Sonnet dry run checked the changes since 1b60fbb: 9 items (1 High, 4 Medium, 4 Low). All were resolved:
- **1 (High):** the hashed lock moved to `tools/requirements.hashes.txt`, so the T-031 lock form is unchanged.
- **2:** R-022 behaviour (no clamping; flag on the processor).
- **3:** `heldValves` as a JSON key; `setHeldValves` added in S-008 and used in S-013.
- **4:** CI additions wired into S-002 actions, Done-when and relevant decisions.
- **5:** artifact name and retention (D-025).
- **6:** stale force-push sentence removed from D-020.
- **7:** order of checkout, install and push (R-020).
- **8:** long-hold tool, output and failure criterion, plus the DR-REGIME trigger.
- **9:** `tokens-revoked` condition in the G-005 trigger.

## Re-run after pre-mortem round 2 hardening (pass 7)
A fresh Sonnet dry run checked the changes since 2892012: 12 items (0 High, 6 Medium, 6 Low). All were resolved:
- **1:** S-017 outputs and done-when cover the G-002/G-006 flow.
- **2:** RC marker `<short sha>` definition, regex, ancestry check and `if:` expression.
- **3:** a G-002 approval covers one sha; re-uploads need a new request.
- **4:** G-002 evidence for the RC request before the upload.
- **5:** `unsupportedRate()` accessor (S-013) and editor overlay (S-014).
- **6:** `tpt_calibrate` read-only modes, render rate, window sampling.
- **7:** p⁺ bound 30 kPa; worst case 238 samples at 192 kHz.
- **8:** nlohmann link and the `heldValves` mechanics.
- **9:** HANDOFF S-017 gates.
- **10:** S-000 credential statement.
- **11:** RC staging, paths and `ImageVersion`.
- **12:** S-008/S-011 outputs include the header and CMake additions.

## Re-run after pre-mortem round 3 hardening (pass 8)
A fresh Sonnet dry run checked the changes since f7c98ab: 10 items (0 High, 5 Medium, 5 Low). All were resolved:
- **1:** counted note stack (D-010).
- **2:** the re-attack is specified exactly as spiked (3 ms, no accent, closed lips), so C-101 applies to T-035.
- **3:** DR-ARTIC.
- **4:** D-012 ring wording; S-011 clamp remedy.
- **5:** `TPT_GIT_SHA` override from the RC job; footer drawn in S-014.
- **6:** D-003 stale clause.
- **7:** `setBreath` header comment (S-010).
- **8:** demo 10 timing.
- **9:** CC11 semantics and the G-006 item 9 wording.
- **10:** licence-text paths and the README "MIDI and latency" section (S-018).

## Re-run after pre-mortem round 4 changes (pass 9)
A fresh Sonnet dry run checked the changes since 257d87d: 5 items (1 High, 2 Medium, 2 Low). All were resolved:
- **1 (High):** the RC sha check is an ancestry check plus a "records only" diff check, replacing `HEAD^`, which the gate
  commits made impossible.
- **2:** CC68 reaches the voice through the additive `setLegatoPedal`; D-010 states the pedal rule; S-010 and S-013 own it.
- **3:** the Windows build needs CMake ≥ 4.2.
- **4:** CC11 and breath reset timing at transport start.
- **5:** README section lists CC68.

No frozen test contradicts CC11 or CC68 (checked by the reviewer: T-016, T-019, T-026, T-035).

## Re-run after the R0 re-run against the real knowledge store (pass 10)
A fresh dry run checked the R0 amendments (diff 104d893..HEAD): 20 items (2 High, 6 Medium, 12 Low). All were resolved:
- **1 (High):** non-frozen tests live under `extras/` (outside the frozen `tests/` tree), wired from the root CMake with the
  fixture define and `tests/core` include, label `extra`, excluded from red-run comparisons (PLAN Conventions).
- **2 (High):** breath noise is a private `BreathNoise.h` class; D-012 states the discrete H(z) and f_int = 2 × host rate;
  the test compares band-power ratios and variance of the filtered noise alone.
- **3–8 (Medium):** `perf_probe` prints the RTF; `tpt::tuning::table()` and `tpt_render --print-tuning`; G-003
  `push-to: knowledge=…, lessons=…`; `--notes` partial-run semantics and full-coverage check; `make_report.py` inputs,
  markers and Done-when; `bootstrap_env.sh` in Outputs, proxy status body only.
- **9–20 (Low):** S-013 Done-when names the variant tests; dry-run sha and regex; narrowed `* 0` grep; session-start
  `git log` once the branch exists; `python3` from the clone root; evidence under `logs/**`; `ui-checklist` in S-014;
  S-RETRO `/tmp/` check; G-006 optional `cmake --version` request; `GIT_CONFIG_*` unset loop and `evidence/push-probe`;
  early fingering reply logged in `EXECUTION_LOG.md`; the READY header replaces IN REVISION (Phase 5).
