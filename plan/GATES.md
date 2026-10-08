# Human gates

At a gate the implementer:
1. halts the gated path;
2. writes `GATE-<id>.md` on the implementation branch, from the template below;
3. pushes;
4. waits for the human's response.

The response is accepted in either of two forms:
- a file `GATE-<id>.RESPONSE.md` committed to the implementation branch;
- the human's message in the session, which the implementer copies verbatim into that file.

While halted, the implementer may continue any step whose `Depends on` list does not include the gated step. It never takes an external or irreversible action.

Template for `GATE-<id>.md`:
- what was done;
- the evidence links (branch, files, CI run URL);
- the questions;
- the allowed responses, verbatim;
- other open items.

## G-001: not applicable
`math` and `computational` profiles are inactive (`plan/PROFILE.md`).

## G-002: external or irreversible action (release, tag, package, merge to `main`)
| Aspect | Definition |
|---|---|
| Trigger | Any step or deviation that would create a tag or GitHub Release, upload binaries anywhere, publish a package, or merge or push to `main`. No step of this plan does any of these. |
| Evidence | The exact command(s), the artefacts, their SHA-256 values, licence implications (AGPLv3 binaries, C-098), and the CI run that built them. |
| Questions | 1. Approve this action exactly as described? |
| Allowed responses | `approve`, `deny`, `approve-with-changes: <text>` |
| Branches | `approve` → perform exactly the listed commands, then record the outcome in `GATE-G-002.md`. `deny` → do nothing; continue. `approve-with-changes` → re-issue the gate with the changes applied. |

## G-003: lessons and knowledge push (always the last step, S-KNOW)
| Aspect | Definition |
|---|---|
| Trigger | S-RETRO done. |
| Evidence | The list of every `lessons/L-*.md` and `knowledge/K-*.md` (ID, title or statement, tags), the schema-check output, and the proposed target `qualitycoding/agent-knowledge` (A-018) and whether it exists. |
| Questions | Where should the run's lessons and knowledge go? |
| Allowed responses | `push-to-proposed`, `push-to: <owner/repo>`, `do-not-push` |
| Branches | Push to the target in the protocol Appendix C layout; regenerate the indexes; create the target if missing (`gh repo create --private`, else the bundle fallback). `do-not-push` → finish. No response within the session → write the bundle fallback `knowledge-<run_id>.bundle` and report its path. |

## G-004: fingering chart and trumpet drawing check (after S-014)
| Aspect | Definition |
|---|---|
| Trigger | S-014 done: the editor works and `tpt_ui_snapshots` has written the screenshots. |
| Evidence (branch `evidence/G-004`, orphan) | `written-<NN>-<name>.png` for all 36 standard fingerings (caption: written and concert note, valves, partial, trigger). `alt-written-<NN>-<name>.png` for the 20 alternates. `overblow-<NN>.png` for 5 notes at Overblow 0.6. `fixed-valves.png`. `editor.png`. Plus a table in `GATE-G-004.md` listing every fingering with valves, partial and trigger, with the choices made where sources differ: C♯6–E6 (C-061), slide triggers (C-064, C-065), alternate list (C-062). |
| Questions | 1. Is every standard fingering correct? 2. Are the alternates the ones you want? 3. Are the slide triggers right (3rd slide on C♯4/D4 only; 1st slide on F5, A5, A♯5)? 4. Is anything drawn wrongly or missing in the trumpet view? |
| Allowed responses | `proceed`, `fix-drawing: <items>`, `correct-chart: <written note>=<valves> p<n> [trigger]; ...`, `stop` |
| Branches | `proceed` → S-016 may start once its other dependencies are met. `fix-drawing` → change the drawing code only (T-033 must still pass); re-issue G-004. `correct-chart` → the frozen fixture is wrong: write `TEST_CHALLENGE.md` (amendment, protocol 2E.4); the planning agent amends the fixture, the manifest and dependent tests (T-003, T-007, T-012); then continue. `stop` → write the final report and stop before S-016. |

## G-005: realism listening sign-off (after S-016)
| Aspect | Definition |
|---|---|
| Trigger | S-016 done: T-029 passes, or the 5 calibration rounds of DR-REAL are exhausted. |
| Evidence (branch `evidence/G-005`, orphan, with `ATTRIBUTION.txt` for TinySOL) | **12 blind pairs:** `blind/pair-NN-A.wav` / `-B.wav`. Each pair holds a TinySOL TpC note and the plugin at the same concert pitch and dynamic, mf steady segment, level-matched, random order. Pitches come from `random.Random(20261008).sample` over the TinySOL mf notes 54–82. The key is `blind/key.json.b64`, base64; don't decode it until you've answered. **Demos:** `demos/01-chromatic-E3-Bb5.wav`, `02-lip-slurs-open-C4-G5.wav`, `03-overblow-sweep-written-C4.wav`, `04-underblow-to-pedal.wav`, `05-fixed-valves-1-3-partials.wav`, `06-dynamics-pp-mf-ff-written-G4.wav`, `07-intonation-realism-0-vs-1.wav`, `08-legato-phrase-vibrato.wav`. Also `SUMMARY.md` (T-029 metrics per dynamic, worst notes) and `results.json`. |
| Questions | 1. Without the key, which file in each pair is the real trumpet? 2. Is the sound realistic enough for your use? 3. Is the Overblow / Underblow / lip-slur behaviour what you wanted (demos 02–05)? 4. Do the natural tendencies at Intonation realism 1 sound right (demo 07; low valved notes, R-005)? |
| Allowed responses | `proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `override-intonation: <written>=<cents>; ...`, `stop` |
| Branches | `proceed` → S-017. `iterate` → up to 3 more DR-REAL rounds following the notes, then re-issue G-005. `proceed-with-rescope` → record a new `A-###` and continue. `override-intonation` → apply DR-INTON (amendment), then continue. `stop` → final report and stop. |

## Gates not required
No other gate is needed: there is no deployment and no publication, and every external action is covered by G-002.
