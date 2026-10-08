# Research round 3 (2026-10-07/08) — R5 adversarial pass, R6 re-runs

- R5 was done by a fresh-context Opus agent that did not write the claims. Its full report is in
  `round-3-adversarial.md`, and its scratch scripts are archived in `research/spikes/adversarial/`.
- The pass was interrupted twice by rate limits and resumed each time.
- The human asked for the attack-parameter grid to be run, so it was run (R6).

## Outcome of R5
| Claim | Verdict | Resolution |
|---|---|---|
| C-044 Fréour residues include Zc | upheld | — |
| C-075 simulator correctness | upheld | Independent RK4 agrees within 0.8 cents. Threshold is 2.3–2.4 kPa against the paper's 2.2 kPa. |
| C-076–C-079 bore and cone matrix | upheld | The cone matrix matches a Webster integration within 1.5e-10. |
| Pins C-001–C-022 | upheld | Breaking changes of the action majors added as C-097. |
| C-073 onset reference | weakened | Superseded by C-096. TinySOL is trimmed at the attack, so there is no reference onset band. D-017 uses an absolute onset bound. The four resampled files are excluded. |
| C-080 regime lock | weakened | Superseded by C-091. Extended-range blowing floors added (D-023). |
| C-081 calibration | weakened | Superseded by C-092. Pitch is valid only for sustained notes, and the tests gate on sustain. |
| C-083 register jumps | weakened | Superseded by C-093. Overblow capped at partial max(n, 9) (D-009). |
| C-086 soft dynamics | weakened in one detail | Superseded by C-094. |
| C-087 pressure match | weakened | Superseded by C-095. The match is about 10 % looser than stated. |
| C-088 attack click | refuted by the planner's own re-run | Superseded by C-089. It was a circular-FFT artefact (lesson `L-20261008T010500Z`). |
| Licensing | finding | NOTICE corrected on `main` (7196310). `ATTRIBUTION.md` added. C-098. |
| Modal calibration like with like | finding | C-099. D-004 requires a complex modal fit and the measured complex residues for the open state. |

## Spike-code defects (from R5) and disposition
| R5 defect | Disposition |
|---|---|
| D-1, D-2 (no sustain gate, short runs) | Tests T-012 and T-015 gate on sustain (2 s renders, held registers). Floors and cap re-verified (`floors_cap_check.py`). |
| D-3, D-4 (onset origin, resampled files) | D-017 / T-029 |
| D-5 (poles vs peaks, real residues) | D-004 |
| D-6 (cache) | Spike only. The implementation generator writes no cache. |
| D-7 (p/2 approximation) | D-012 uses p⁺ = (p + Zc·u)/2 in the voice. |
| D-8 (Z0 > 0) | D-004 validation requires Re(R) > 0. T-006 case added. |
| D-9 (comment) | Spike only, cosmetic. |

## Open questions carried into the plan
| Question | Disposition |
|---|---|
| YIN (shipped) vs ACF (spikes) | S-004 cross-checks YIN against the spike decisions on the regime map. The T-010 weak-fundamental case was added. |
| Natural tendencies of p2 valved notes: the model says flat, some teaching sources say sharp | R-005. Human judgement at G-004/G-005. Decision rule DR-INTON. |
| Modes 12–14 reuse the mode-11 height ratio | R-007. Only the extended range and overblow above p11 depend on them, and overblow is capped at p9. |
