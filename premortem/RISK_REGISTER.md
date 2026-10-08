# Risk register

Severity uses the Global Rule 4 rubric (software profile). Likelihood is L, M or H. This version holds the planner's
initial entries from research; pre-mortem rounds add entries and mitigations (`premortem/round-N.md`).

| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-001 | Realism thresholds (T-029) not met after 5 calibration rounds: pp too bright, ff too dark in places | technical / research | Medium | M | The physical model is simplified (one-mass lips, output-side nonlinearity, approximated p⁺); spike margins at pp are thin (4.3–6.6 dB vs 6 dB) | C-085, C-094, D-012, T-029 | DR-REAL: 5 rounds on listed constants; the human decides at G-005 (`iterate` or `proceed-with-rescope`); thresholds never edited | open (accepted path via G-005) |
| R-002 | Regime lock or tuning lost after the generator is changed from the spike's like-for-like-less calibration to complex modal fit (C-099) | technical | Medium | M | Spike results (C-091/C-092) used the draft table | D-004, T-012, T-013 | DR-REGIME per-partial trims; calibration with sustain gate; T-012/T-013 frozen | open |
| R-003 | TinySOL is a C trumpet, compared at concert pitch with a B♭ model (different partial/valve state) | research | Medium | H | Only CC-licensed trumpet set found (C-055) | T-029, C-096 | Thresholds from the reference's own spread; G-005 blind test is the arbiter | accepted |
| R-004 | Lip-stiffness mapping (h0 × 1.15 − 0.3 s) not spiked; extremes may break lock | technical | Medium | M | Lesson L-…150600 only partly applied | D-005, T-012 | T-012 tests both extremes; DR-REGIME allows narrowing the h0 span in `VoiceTuning.h` (logged), not the test | open |
| R-005 | Natural tendencies at Intonation realism = 1 for partial-2 valved notes (model: 35–62 c flat) disagree with some teaching sources (sharp) | research | Low | M | No measured valved impedance (C-079, R5 question 3) | D-011, T-014 | G-005 question 4; DR-INTON override path | open |
| R-006 | Attack onset only partly calibrated: grid coverage of the trimmed-reference band 25–92 %; absolute median bound only | technical | Low | M | The reference onsets are unusable (C-096) | D-007, T-029 | Median ≤ 150 ms frozen (spike medians 47–102 ms); human listening G-005 | open |
| R-007 | Modes 12–14 reuse the mode-11 calibration ratios; notes on p12 (written 89; alternates 87, 88) depend on extrapolated data | research | Low | M | Measured data end at mode 11 | D-004, C-099 | Extended range only; Overblow capped at p9; T-012 checks that they sustain | open |
| R-008 | Pedal tones started directly in Fixed-valves mode (partial 1) are untested | technical | Low | M | Spikes covered only Underblow into p1 | D-003 | Documented; Underblow path tested (T-015) | accepted |
