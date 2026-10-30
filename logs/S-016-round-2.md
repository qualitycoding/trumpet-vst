# S-016 round 2 (G-005 `iterate`: the attack is too clean)

Owner's note: the real clips are identified by the very start of the clip. The blind clips are the 1.0 s steady segments, which start 0.15 s after the onset.
Measurements on the 12 blind pairs and on all TinySOL notes (scripts in the session scratchpad; numbers below are printed by them):

Before (round 1): the synthetic notes were perfectly steady in pitch (std 0.0 cents after 0.4 s, jitter 0.03 cents against 0.47 cents for the real notes),
started 10 cents sharp inside the clip, sounded sharp by 70-190 cents for up to 200 ms after the onset (the oscillation starts at the lip frequency), and reached
their level too early (no crescendo after the clip start: real clips rise 5 dB over their first 200 ms, the synthetic ones 2 dB).

## Constants changed this round (diff of `logs/S-016/tuning-round-1.json` and `-2.json`)
- `kAttackNoise`: (new) -> 4
- `kAttackNoiseS`: (new) -> 0.05
- `kBloomDb`: (new) -> 3
- `kBloomS`: (new) -> 0.25
- `kCreepDepth`: (new) -> 0.35
- `kCreepTauS`: (new) -> 0.25
- `kFfShelfDb`: 8 -> 10
- `kFlTrim1`: 1 -> 1.03
- `kJitterCents`: (new) -> 0.2
- `kJitterHz`: (new) -> 40
- `kScoopCents`: (new) -> -150
- `kScoopFfCut`: (new) -> 0.7
- `kScoopS`: (new) -> 0.07
- `kTongueStopS`: 0.028 -> 0.036
- `kWanderAttCents`: (new) -> 20
- `kWanderHz`: (new) -> 3
- `kWanderSsCents`: (new) -> 2
- `kWanderTauS`: (new) -> 0.2
- `kFlTrim` partial 1: 1 -> 1.03 and `kTongueStopS` 0.028 -> 0.036 (DR-REGIME / DR-ARTIC: with the wander the underblow to the pedal tone and the
  T-035 recovery count were borderline; fl x 1.03 on partial 1 gave 0 failures over four wander seeds, the other trims did not).

What they are: a pitch wander of the player (slow low-passed noise, std 2 cents in the sustain and +20 cents at the note start, decaying with 0.2 s; fast jitter 0.2 cents),
a lip-frequency scoop at a tongued note start (-150 cents, 70 ms, 30 % of it at ff), an output-level bloom (-35 % gain at the start, 0.25 s) and a
high-shelf bloom (-3 dB at the start, 0.25 s). Breath-noise burst at the attack (`kAttackNoise`) and a pressure creep were tried; the first had no effect,
the second broke the sensitive notes and moved to the output gain.

## Result at the clip start (blind-pair clips, from `renders/G-005`)
```
segment      level dB (real/syn) | share of energy >5 kHz dB (real/syn)
0-20ms       -27.6  -26.6 |  -39.2  -32.4
20-60ms      -25.5  -24.1 |  -39.1  -31.0
60-200ms     -24.4  -23.8 |  -37.8  -31.3
200-1000ms   -22.7  -22.8 |  -32.7  -30.8
```
```
clip time   mean cents rel. to the last 300 ms   (real | syn)   and std over notes
   0 ms     -2.5    2.4 | std  10.2   9.7
  20 ms     -2.2    2.9 | std   9.6   4.8
  50 ms     -2.2    3.6 | std   8.2   8.4
 100 ms     -1.4   -7.0 | std   7.0   2.8
 200 ms      0.2   -4.4 | std   4.3   2.4
 400 ms      0.6    0.8 | std   2.7   0.7
 600 ms      0.5    1.4 | std   1.8   0.4
 800 ms      0.4    2.1 | std   0.6   1.4
abs deviation in first 100 ms: real 6.8 cents, syn 6.1 cents
jitter (std of 10 ms-to-10 ms differences after 200 ms): real 0.47 syn 0.69 cents
```

## T-029
# Realism summary (T-029 metrics, D-017)

82 rows.

| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |
|---|---|---|---|---|---|---|
| pp | 29 | 5.19 | 93 % | 97 % | 68 | pass |
| mf | 28 | 3.66 | 100 % | 96 % | 79 | pass |
| ff | 25 | 2.73 | 100 % | 96 % | 48 | pass |

Centroid ordering pp < mf < ff: 24 of 24 pitches (pass).


All frozen suites pass locally: unit, integration (T-012..T-022, T-034, T-035), operational, alloc, perf; long hold has no failing note; brightness-ff has no clamp event.
