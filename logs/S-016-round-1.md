# S-016 round 1 (DR-REAL)

Input: S-015 baseline (`logs/S-015/SUMMARY.md`): pp and mf passed, ff failed the centroid band (0 %), T-022 24/31.

## Constants changed this round (printed by `tpt_render --print-tuning`, diff of `logs/S-016/tuning-round-0.json` and `-1.json`)
- `kFfShelfDb`: (new) -> 8
- `kFfShelfHz`: (new) -> 1800
- `kFfShelfQ`: (new) -> 0.7
- `kNlFfBoost`: (new) -> 0
- `kOutGain`: 2.222222222e-05 -> 1.086956522e-05
- `kPpShelfDb`: (new) -> 8

Rationale: the synthetic ff spectrum lacked 2.6 dB at 2-4 kHz and 6 dB at 4-8 kHz relative to TinySOL, while mf was right.
A static shelf fixed ff but broke pp/mf, so the brightness depends on the blowing level: a high shelf at 1.8 kHz
(Q 0.7) with +8 dB at ff (L = 1), 0 dB from mf (L <= 0.6) down, and -8 dB at pp (L <= 0.2), linear in L between.
K_out: written C5 at ff now peaks at -6.0 dBFS (0.501). A level-dependent L_nl (`kNlFfBoost`, tried 1..4) and a higher ff
blowing factor (`kSusHigh` 6, 7.5) were tested and did not help: the centroid band saturated at 36 %; both stay at their
original values (`kNlFfBoost` 0, `kSusHigh` 5).

## Results
# Realism summary (T-029 metrics, D-017)

82 rows.

| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |
|---|---|---|---|---|---|---|
| pp | 29 | 5.03 | 97 % | 97 % | 98 | pass |
| mf | 28 | 4.00 | 100 % | 96 % | 82 | pass |
| ff | 25 | 2.64 | 100 % | 88 % | 65 | pass |

Centroid ordering pp < mf < ff: 24 of 24 pitches (pass).


T-021 and T-022 (integration suite): pass (T-022 brassy 27/31, was 24/31). T-012..T-020, T-034, T-035, operational,
alloc and perf suites: pass. Long hold: no failing note. Brightness-ff: 0 clamp events. RTF 0.0106.
