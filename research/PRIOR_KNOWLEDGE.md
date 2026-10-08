# Prior knowledge (R0) and how it is applied

## Aliases
"R5" and "R5 review" refer to the round-3 adversarial review, `research/rounds/round-3-adversarial.md`; "R5 D-n" is its
defect n (section 2) and "R5 question n" its section 5 item n.

## Sources
The knowledge store `qualitycoding/agent-knowledge` does not exist (HTTP 404, 2026-10-07), so nothing was loaded from
it. The sibling runs clarinet-vst, saxophone-vst and chinese-strings recorded no lessons. Their problems were backfilled
as planner lessons (A-019) and are loaded here, together with the planner's own lessons from this run.

## Prevention-rule checklist (the cold read and the pre-mortem work from this list)
Order: severity, then most recent.

| # | Lesson | Prevention rule |
|---|---|---|
| 1 | High — L-20261008T010500Z-circular-fft-filter-fakes-onsets | Zero-pad spectral filtering of audio whose time structure is measured. |
| 2 | High — L-20261007T150300Z-hand-typed-frozen-constant | Generate every numeric expected value in a frozen test with a committed script. |
| 3 | High — L-20261007T150400Z-threshold-not-checked-against-reference-spread | Show that a plausible prediction passes each reference-comparison threshold before freezing. |
| 4 | High — L-20261007T150500Z-linear-bore-realism-ceiling | Spike the architecture against the reference recordings (3 notes × 3 dynamics) before freezing thresholds. |
| 5 | High — L-20261007T150600Z-control-mapping-formulas-unverified | Run every control→model formula in a spike over its full range; the default must equal the verified operating point. |
| 6 | High — L-20261007T150700Z-regime-selection-per-note | Simulate every (fingering, intended mode) pair at the default setting before freezing. |
| 7 | Medium — L-20261007T150000Z-sandbox-no-pip-no-sudo | Use a `--without-pip` venv + get-pip and release binaries; never assume apt or pip. |
| 8 | Medium — L-20261007T150100Z-no-credentials-in-command-lines | Never expand a credential into a command line, URL, header or file. |
| 9 | Medium — L-20261007T150800Z-host-facing-accessors-missing | Give every value the host layer needs an accessor before freezing the headers. |
| 10 | Low — L-20261007T150200Z-restart-means-fresh-state | On "Restart", use only the repository and the conversation. |
| 11 | Low — L-20261007T150900Z-windows-sha256sum-backslash | Hash via stdin in frozen bash scripts. |
| 12 | Low — L-20261007T151000Z-large-evidence-off-impl-branch | Put audio gate evidence on an orphan evidence branch. |

## Application
| Lesson | Applied as |
|---|---|
| L-20261008T010500Z | Spikes fixed and re-run (C-089). T-028 checks that a slower attack measures later. S-004 and S-005 cite the rule. |
| L-20261007T150300Z | `research/spikes/expected_values.py`. Every float literal in frozen tests carries an `EXPECTED:` tag. T-032 runs `--check`. S-003 generates the fingering table by script. |
| L-20261007T150400Z | `ref_spread.py` (C-073), corrected by C-096: the onset reference is invalid, so the absolute bound uses the spike medians (C-089). T-029 docstring records feasibility for every threshold. |
| L-20261007T150500Z | `timbre_spike.py` and `dyn_spike.py` (C-085, C-094) led to adding nonlinear propagation (D-012) before freezing. Residual risk R-001. |
| L-20261007T150600Z | Overblow mapping spiked over its range (C-082–C-084, C-093). Dynamics map spiked (C-086, C-094). Attack spiked (C-089). The lip stiffness mapping (D-005) was **not** spiked → T-012 tests both extremes, risk R-004. |
| L-20261007T150700Z | `regime_map.py` plus the R5 4 s re-runs (C-091). T-012 has a sustain gate. S-009 calibration gates on sustain. |
| L-20261007T150000Z | `plan/ENVIRONMENT.md` no-sudo commands; S-000. |
| L-20261007T150100Z | HANDOFF credential rule; S-000; S-KNOW secret scan. |
| L-20261007T150800Z | `latencySamples()`, `uiState()`, `soundingHz()`, `keyswitch()` and `currentUiState()` are in the frozen headers (D-013); S-006/S-008/S-013 log any additive accessor. |
| L-20261007T150200Z | Not applicable to plan steps (a planner process rule); restated in the HANDOFF halt protocol for restarts. |
| L-20261007T150900Z | `tests/scripts/run_pluginval.sh` hashes via stdin. Verified on windows-2025 (`plan_verify_ci.md`). |
| L-20261007T151000Z | D-019 evidence branches. S-014 and S-016 use orphan branches. `.gitignore` excludes renders and WAVs. |

## Knowledge items used
None from the store.

This run's items are `knowledge/K-20261007T16*.md`, generated from `research/claims.json` by
`research/spikes/build_claims.py`. Seven of them are marked superseded: C-073, C-080, C-081, C-083, C-086, C-087 and
C-088.
