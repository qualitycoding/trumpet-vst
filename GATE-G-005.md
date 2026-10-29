# GATE-G-005: realism listening sign-off

## What was done
- S-016 finished after one calibration round (DR-REAL, `logs/S-016-round-1.md`): T-029, T-021 and T-022 all pass; no
  accepted failures. Constants changed: a level-dependent high shelf (+8 dB at ff, -8 dB at pp, 1.8 kHz), and K_out
  (written C5 at ff peaks at -6.0 dBFS). `DEVIATIONS.md` explains why.
- The bundle was built by the CI job `gate-evidence` (marker commit `[gate-evidence G-005]`).

## Evidence
- Branch `evidence/G-005` (https://github.com/qualitycoding/trumpet-vst/tree/evidence/G-005), commit 82329ff, with `ATTRIBUTION.txt`
  for the TinySOL excerpts; SHA-256 list on this branch: `gates/G-005/SHA256SUMS` (verified with `sha256sum -c`).
- CI run: https://github.com/qualitycoding/trumpet-vst/actions/runs/40873290014 (job `gate-evidence`).
- **12 blind pairs** `blind/pair-NN-A.wav` / `-B.wav` (44.1 kHz, 24-bit; 1.0 s steady segments at -23 dBFS RMS, 10 ms fades); the key is
  `blind/key.json.b64` (base64 of `{"pair-01": "A", ...}`, naming the real file). Do not decode it until you have answered.
- **Demos** (`demos/*.wav`, rendered by `tpt_render --events tools/realism/demos/<name>.json` at 48 kHz): 01 chromatic E3-Bb5, 02 lip slurs,
  03 overblow sweep, 04 underblow to the pedal tone, 05 fixed valves 1-3, 06 dynamics pp/mf/ff, 07 intonation realism 0 vs 1,
  08 legato phrase with vibrato, 09 brightness 0 / 0.5 / 1 at ff, 10 articulation.
- `SUMMARY.md` and `results.json` (T-029 metrics per dynamic):

82 rows.

| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |
|---|---|---|---|---|---|---|
| pp | 29 | 5.03 | 97 % | 97 % | 98 | pass |
| mf | 28 | 4.00 | 100 % | 96 % | 82 | pass |
| ff | 25 | 2.64 | 100 % | 88 % | 65 | pass |

Centroid ordering pp < mf < ff: 24 of 24 pitches (pass).


## Questions
1. Without the key, which file in each pair is the real trumpet?
2. Is the sound realistic enough for your use?
3. Is the Overblow / Underblow / lip-slur behaviour what you wanted (demos 02-05)? Is brightness 1 at ff free of crackle (demo 09)? Do repeated and detached notes articulate, and legato notes slur (demo 10)?
4. Do the natural tendencies at Intonation realism 1 sound right (demo 07; low valved notes, R-005)? Note: trigger notes (C#4, D4, F5, A5, A#5) keep their slide correction at realism 1, so they deviate only by the harmonic-series amount.

## Allowed responses (verbatim)
`proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `override-intonation: <written>=<cents>; ...`, `stop` (`proceed` while T-029/T-021/T-022 are red accepts those failures: they stay red and are listed in `REPORT.md`, S-017/S-018)

## Other open items
- **`tokens-revoked` has not been recorded** (your G-004 answer was `proceed` only). G-005 cannot be processed without it: please revoke the three GitHub tokens pasted in chat during planning and add `tokens-revoked` to your answer.
- Gate G-002 (uploading release-candidate binaries from this public repository) and G-006 (REAPER host test) follow in S-017.
- CI state at this commit: `freeze`, `core` (three OSes), `python`, `build-linux`, `realism` green; the `plugin` jobs were still running when this file was written (they were green on the previous commit).
