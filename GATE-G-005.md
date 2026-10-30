# GATE-G-005 (round 2): realism listening sign-off

## What was done since round 1
- Your answer was `iterate`: the attack is too clean; you recognised the real sounds by the very start of each clip (10 of 12 right).
- I measured the start of the blind clips (they begin 0.15 s after the onset) on the 12 pairs and on all TinySOL notes. The real notes wander in pitch by
  several cents just after the onset and keep a fine jitter, start about 2.5 dB lower inside the clip and bloom for another 0.2 s; the synthetic
  notes were perfectly steady in pitch, started sharp, and had no bloom. Details and numbers: `logs/S-016-round-2.md`.
- DR-REAL round 2 added a player-instability model (pitch wander and jitter, strongest just after the onset), a lip-frequency scoop at tongued
  note starts, a level bloom and a brightness bloom after the attack, and a short breath-noise burst. T-029, T-021, T-022 and every other frozen test still pass.
- The blind pairs use a **new random draw** (seed 20261030), because the key of round 1 became known when I told you which pairs you had right.

## Evidence
- Branch `evidence/G-005` (https://github.com/qualitycoding/trumpet-vst/tree/evidence/G-005), commit 00d9ef1, `ATTRIBUTION.txt` included; SHA-256 list:
  `gates/G-005/SHA256SUMS` (verified with `sha256sum -c`). CI run: https://github.com/qualitycoding/trumpet-vst/actions/runs/41046024094 (job `gate-evidence`).
- 12 blind pairs `blind/pair-NN-A.wav` / `-B.wav` (44.1 kHz, 24 bit, 1.0 s steady segments at -23 dBFS RMS, 10 ms fades), key `blind/key.json.b64` (base64; not decoded by me).
- The ten demos in `demos/`, `SUMMARY.md` and `results.json`. T-029 metrics:

82 rows.

| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |
|---|---|---|---|---|---|---|
| pp | 29 | 5.19 | 93 % | 97 % | 68 | pass |
| mf | 28 | 3.66 | 100 % | 96 % | 80 | pass |
| ff | 25 | 2.73 | 100 % | 96 % | 69 | pass |

Centroid ordering pp < mf < ff: 24 of 24 pitches (pass).


## Questions
1. Without the key, which file in each pair is the real trumpet? (Please say again which clips gave it away, if you can still tell.)
2. Is the sound realistic enough for your use?
3. Is the Overblow / Underblow / lip-slur behaviour what you wanted (demos 02-05)? Is brightness 1 at ff free of crackle (demo 09)? Do repeated and detached notes articulate, and legato notes slur (demo 10)?
4. Do the natural tendencies at Intonation realism 1 sound right (demo 07; low valved notes, R-005)? Note: trigger notes (C#4, D4, F5, A5, A#5) keep their slide correction at realism 1, so they deviate only by the harmonic-series amount.

## Allowed responses (verbatim)
`proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `override-intonation: <written>=<cents>; ...`, `stop` (`proceed` while T-029/T-021/T-022 are red accepts those failures: they stay red and are listed in `REPORT.md`, S-017/S-018)

## Other open items
- `tokens-revoked` is recorded (your message of 2026-10-30, "The pat has been revoked"; singular, see `GATE-G-004.RESPONSE.md`).
- Iteration count: this is round 2 of at most 4 (the plan allows 3 more rounds after the first G-005); round 1 (S-016) was the first.
- Gates G-002 (upload of release-candidate binaries from this public repository) and G-006 (REAPER host test) follow in S-017.
