# Research round 2 (2026-10-07) — R3 synthesis, R4 depth, R6 spikes

## R3 Synthesis and gaps
| Gap / contradiction | Resolution |
|---|---|
| Two trumpet lip parameter families: Fréour 2020/2022 (Ql 3, mu 2, f_l ≈ 0.82 f_res) vs Doc 2023 (Ql 20, mu 9, f_l ≈ f_res, per-register h0) | Both reproduce oscillation; Doc 2023 chosen (per-register table, thresholds measured on players, register selection demonstrated in synthesis) — D-005. Fréour set kept as the reference-integrator check (T-009). |
| No measured valved impedances; no measured valve loop lengths (C-049) | Own parametric bore fitted to Fréour's open-horn poles (C-076), loops tuned in the TMM (C-079), open-horn calibration ratios (C-077). Uniform scaling rejected (C-078). |
| Fingering above C6 disagrees with the planner's draft (C-061) | Adopt the 2-source consensus (C♯6 = 2 p9, D6 = 0 p9, D♯6 = 2 p10, E6 = 0 p10, F6 = 1 p12); human check at G-004. |
| Slide triggers: F♯3/G3 already low on many horns (C-064) | 3rd-slide trigger only for C♯4 and D4; 1st-slide trigger for F5, A5, A♯5 (C-065); TMM agrees qualitatively (C-079). |
| Arithmetic "inferred" claims (C-054, C-056, C-069) | Verified by computation (`expected_values.py`). |
| Radiation shape single-source (C-051) | Transfer computed by the TMM instead (D-012). |
| auval / xvfb only inferred (C-026, C-029) | Corroborated by the clarinet-vst CI on the same JUCE version. |
| C-trumpet recordings as reference for a B♭ model (C-055 inferred) | Kept as reference (only CC-licensed trumpet set found); thresholds derived from the reference's own spread; human blind test G-005 is the arbiter; risk R-003. |

## R4 Depth
Tier-1 locators read directly by the planner: Velut 2017 eq. (1), (3)–(5), Table 2; Fréour 2022 eq. (1), Table 1,
Table 2, p. 4–6; Doc 2023 Table I and II, Section V; JUCE 9.0.3 tag, LICENSE, CMakeLists; Zenodo record JSON.

## R6 Spikes (all committed with outputs)
| Spike | Claim |
|---|---|
| `bore_fit.py` | C-076 |
| `make_table.py` | C-077, C-079 |
| `tmm_trumpet.py` (scaling comparison) | C-078 |
| `regime_map.py` | C-080, C-081 |
| `overblow_spike.py` | C-082 |
| `overblow_d.py` | C-083 |
| `brighten_spike.py` | C-084 |
| `timbre_spike.py` | C-085 |
| `dyn_spike.py` | C-086 |
| `regime_map.py` (pressures) | C-087 |
| `onset_spike.py` | C-088 |

A grid search over attack parameters (`onset_grid.py`, written but not run: the human declined the run) was not
performed; the attack is calibrated during implementation against the frozen onset band (D-007, risk R-006).

## Outcome
New load-bearing claims this round: 16 (all verified by spikes). Confidence downgrades: none. Open contradictions:
none. → Round 3 = adversarial pass (R5) + saturation check.
