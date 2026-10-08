# Cold-read gate log (protocol 3.6)

| Pass | Agents (fresh context) | Items | Result |
|---|---|---|---|
| 1 | Sonnet dry run (interrupted twice by rate limits; 50 items recorded before the final stop) + Haiku mechanical check | 50 dry-run items (7 High, 23 Medium, 20 Low); mechanical: 8/8 checks PASS | All 50 resolved in commit 196ec49 (see below) |
| 2 | New Sonnet dry run (report complete, 28 items, then rate-limited) + new Haiku mechanical check | 28 items (3 High, 8 Medium, 17 Low); mechanical: 0 failures | All 28 resolved (see below) |

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
