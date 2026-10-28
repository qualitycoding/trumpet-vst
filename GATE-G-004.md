# GATE-G-004: fingering chart and trumpet drawing check

## What was done
- S-014 is complete: the editor (960x600, 9 knobs, 2 toggles, build id), the trumpet drawing and the layout are implemented;
  T-033 and all of T-026 pass (CI plugin jobs green on Linux, macOS and Windows).
- `tpt_ui_snapshots` was run by the CI job `gate-evidence` (marker commit `[gate-evidence G-004]`) and the 63 PNGs were pushed
  to the orphan branch `evidence/G-004`.

## Evidence
- Branch: `evidence/G-004` (https://github.com/qualitycoding/trumpet-vst/tree/evidence/G-004), commit d81d071; SHA-256 list:
  `gates/G-004/SHA256SUMS` on the implementation branch (verified: `sha256sum -c` passes on the downloaded files).
- CI run: https://github.com/qualitycoding/trumpet-vst/actions/runs/40785962043 (job `gate-evidence`).
- Files: `written-<NN>-<name>.png` (36 standard fingerings), `alt-written-<NN>-<name>.png` (20 alternates),
  `overblow-<NN>.png` for written 60, 67, 72, 79, 84 (target partial n + 1), `fixed-valves.png` (valves 1-3 held, idle),
  `editor.png` (written 62 = 1-3 p3, 3rd slide). The images are built from `resolveNote`; the voice does not run.
- UI checklist (every D-013 and S-014 item built or deviated, none blank): `logs/S-014/ui-checklist.md`.

## Fingerings
The table below is the frozen fixture `tests/fixtures/trumpet_fingerings.txt` (columns: written MIDI, note name, std/alt, valves, partial, trigger).
Choices where sources differ: C#6-E6 follows the two-source consensus (C-061); the 3rd slide only on C#4 and D4, the 1st slide on F5, A5 and A#5 (C-064, C-065);
the alternate list is C-062 (the alternate G#5 on 1-2-3 partial 9 was dropped because it cannot be calibrated, C-081).

| written | name | kind | valves | partial | trigger |
|---|---|---|---|---|---|
| 54 | F#3 | std | 123 | 2 | - |
| 55 | G3 | std | 13 | 2 | - |
| 56 | G#3 | std | 23 | 2 | - |
| 57 | A3 | std | 12 | 2 | - |
| 58 | A#3 | std | 1 | 2 | - |
| 59 | B3 | std | 2 | 2 | - |
| 60 | C4 | std | 0 | 2 | - |
| 61 | C#4 | std | 123 | 3 | 3 |
| 62 | D4 | std | 13 | 3 | 3 |
| 63 | D#4 | std | 23 | 3 | - |
| 64 | E4 | std | 12 | 3 | - |
| 65 | F4 | std | 1 | 3 | - |
| 66 | F#4 | std | 2 | 3 | - |
| 67 | G4 | std | 0 | 3 | - |
| 68 | G#4 | std | 23 | 4 | - |
| 69 | A4 | std | 12 | 4 | - |
| 70 | A#4 | std | 1 | 4 | - |
| 71 | B4 | std | 2 | 4 | - |
| 72 | C5 | std | 0 | 4 | - |
| 73 | C#5 | std | 12 | 5 | - |
| 74 | D5 | std | 1 | 5 | - |
| 75 | D#5 | std | 2 | 5 | - |
| 76 | E5 | std | 0 | 5 | - |
| 77 | F5 | std | 1 | 6 | 1 |
| 78 | F#5 | std | 2 | 6 | - |
| 79 | G5 | std | 0 | 6 | - |
| 80 | G#5 | std | 23 | 8 | - |
| 81 | A5 | std | 12 | 8 | 1 |
| 82 | A#5 | std | 1 | 8 | 1 |
| 83 | B5 | std | 2 | 8 | - |
| 84 | C6 | std | 0 | 8 | - |
| 85 | C#6 | std | 2 | 9 | - |
| 86 | D6 | std | 0 | 9 | - |
| 87 | D#6 | std | 2 | 10 | - |
| 88 | E6 | std | 0 | 10 | - |
| 89 | F6 | std | 1 | 12 | - |
| 57 | A3 | alt | 3 | 2 | - |
| 64 | E4 | alt | 3 | 3 | - |
| 66 | F#4 | alt | 123 | 4 | - |
| 67 | G4 | alt | 13 | 4 | - |
| 69 | A4 | alt | 3 | 4 | - |
| 72 | C5 | alt | 23 | 5 | - |
| 73 | C#5 | alt | 123 | 6 | - |
| 74 | D5 | alt | 13 | 6 | - |
| 75 | D#5 | alt | 23 | 6 | - |
| 76 | E5 | alt | 12 | 6 | - |
| 78 | F#5 | alt | 123 | 8 | - |
| 79 | G5 | alt | 13 | 8 | - |
| 81 | A5 | alt | 3 | 8 | - |
| 82 | A#5 | alt | 23 | 9 | - |
| 83 | B5 | alt | 12 | 9 | - |
| 84 | C6 | alt | 1 | 9 | - |
| 85 | C#6 | alt | 12 | 10 | - |
| 86 | D6 | alt | 1 | 10 | - |
| 87 | D#6 | alt | 23 | 12 | - |
| 88 | E6 | alt | 12 | 12 | - |

## Questions
1. Is every standard fingering correct?
2. Are the alternates the ones you want?
3. Are the slide triggers right (3rd slide on C#4/D4 only; 1st slide on F5, A5, A#5)?
4. Is anything drawn wrongly or missing in the trumpet view?

## Allowed responses (verbatim)
`proceed`, `fix-drawing: <items>`, `correct-chart: <written note>=<valves> p<n> [trigger]; ...`, `stop`; each may be followed by `tokens-revoked` (see Open human actions)

## Other open items
- The three GitHub tokens pasted in chat during planning must be revoked (reply `tokens-revoked` together with your answer; G-005 is not processed until it is recorded).
- Deviation: there is no level meter in the editor, only a "sounding" lamp (`DEVIATIONS.md` S-014, `logs/S-014/ui-checklist.md`).
- Current realism state (S-015): pp and mf pass the T-029 thresholds, ff fails the centroid band; S-016 works on it (G-005 follows).
