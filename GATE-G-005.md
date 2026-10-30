# GATE-G-005 (round 3): realism listening sign-off

## What was done since round 2
- Your answers after round 2: only pairs 6 and 9 could be told apart (both picked correctly); the demos sound right except the underblow, whose pedal tone sounds
  synthetic, and the overblow, which is less bright and harsh than expected. You chose `iterate`.
- Pedal tone (demo 04): the voice played the pedal on the first resonance of the table (83 Hz), so it sounded five semitones flat (85-95 Hz instead of Bb2 at 116 Hz) and weak.
  The lip now sits on the harmonic series of the upper resonances: the pedal sounds at 115 Hz, 13 dB louder, and wanders more like a real pedal tone.
- Overblow (demo 03): overblowing now also makes the note brighter and harsher, as when a player blows harder: at Overblow 1 the note has the brightness of ff
  and twice the nonlinear steepening (share of energy above 2 kHz 50 % instead of 11-16 %; level +8 dB over the sweep). The blowing pressure, and so the register behaviour, is unchanged.
- T-029, T-021, T-022 and every other frozen test still pass. Details and numbers: `logs/S-016-round-3.md`.
- The 12 blind pairs use a new random draw (seed 20261031).

## Evidence
- Branch `evidence/G-005` (https://github.com/qualitycoding/trumpet-vst/tree/evidence/G-005), commit 491171e; SHA-256 list: `gates/G-005/SHA256SUMS` (verified with `sha256sum -c`).
  CI run: https://github.com/qualitycoding/trumpet-vst/actions/runs/41052907508 (job `gate-evidence`).
  Download everything: https://github.com/qualitycoding/trumpet-vst/archive/refs/heads/evidence/G-005.zip
- 12 blind pairs `blind/pair-NN-A.wav` / `-B.wav` (44.1 kHz, 24 bit), key `blind/key.json.b64` (not decoded by me), the ten demos in `demos/`, `SUMMARY.md`, `results.json`. T-029 metrics:

82 rows.

| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |
|---|---|---|---|---|---|---|
| pp | 29 | 5.50 | 93 % | 97 % | 65 | pass |
| mf | 28 | 3.22 | 100 % | 96 % | 78 | pass |
| ff | 25 | 2.12 | 100 % | 96 % | 78 | pass |

Centroid ordering pp < mf < ff: 24 of 24 pitches (pass).


## Questions
1. Without the key, which file in each pair is the real trumpet?
2. Is the sound realistic enough for your use?
3. Is the Overblow / Underblow / lip-slur behaviour what you wanted (demos 02-05)? Is the pedal tone of demo 04 better, and the overblow of demo 03 brighter and harsher? Is brightness 1 at ff free of crackle (demo 09)? Do repeated and detached notes articulate, and legato notes slur (demo 10)?
4. Do the natural tendencies at Intonation realism 1 sound right (demo 07; low valved notes, R-005)?

## Allowed responses (verbatim)
`proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `override-intonation: <written>=<cents>; ...`, `stop` (`proceed` while T-029/T-021/T-022 are red accepts those failures: they stay red and are listed in `REPORT.md`, S-017/S-018)

## Other open items
- `tokens-revoked` is recorded (see `GATE-G-004.RESPONSE.md`).
- Iteration count: round 3 (two `iterate` answers so far); at most 3 `iterate` rounds are allowed after the first G-005, so one more is possible.
- G-002 (upload of release-candidate binaries) and G-006 (REAPER host test) follow in S-017.
