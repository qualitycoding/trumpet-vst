# Pre-mortem round 3 (planning protocol Phase 4)

- Reviewer: fresh-context pre-mortem agent (round 3). I wrote neither the plan nor rounds 1–2. The review is read-only,
  except for this file.
- Plan reviewed: `gen-20261007T145241Z-trumpet-vst-plan` at `f7c98ab` (the cold read of the round-2 changes, 12 items:
  RC marker semantics, approval scope, `unsupportedRate()`, read-only calibrate modes, the D-012 warp bound, outputs).
  `bash tests/scripts/verify_freeze.sh` printed `freeze OK`. The working tree is clean.
- Inputs: `HANDOFF.md`, `plan/*.md`, `premortem/round-1.md`, `premortem/round-2.md`, `premortem/RISK_REGISTER.md`
  (R-001..R-028), `research/PRIOR_KNOWLEDGE.md`, all 14 `lessons/`, `research/claims.json` (C-001..C-100),
  `research/coldread/pass-7-dryrun.md`, the frozen tests in `tests/`, the plugin and core headers, `plugin/CMakeLists.txt`,
  `LICENSE`, `NOTICE`, `ATTRIBUTION.md` and the spikes used below.
- Independent checks, all in the session scratchpad. Nothing was committed.
  - **Warp offsets in both directions (R-028).** `research/spikes/lipsim.cpp` built with `g++ -O2 -std=c++20`, driven by a
    pure-Python port of `regime_map.sim()` (this sandbox has no numpy: lesson #7 again), with `table_draft.json` and the
    `regime_map_result.json` fscale. Written 60, 69, 78, 81 and 84 at mf (2.5 × threshold) and ff (4.8 ×), 1.2 s; p⁺ = p/2
    as in `timbre_spike.py:70`; offset = β·L_nl·p⁺/(ρc³)·f_int with L_nl = 1.7 m (brightness 1). Results in §3, R-028.
  - **Release-tail length (R-029).** Modal T60 = ln(1000)/|Re s| of the open state in `research/spikes/table_draft.json`:
    modes 1–8 give 423, 267, 220, 183, 163, 140, 122 and 106 ms.
  - Secret scan of the tree and of `git log --all -p` for `ghp_`/`github_pat_`/`gho_` patterns: clean.

---

## 1. Incident report (simulation: 6 months after handover)

**Title:** The trumpet cannot tongue. Repeated notes merge into one tone and detached phrases come out slurred, so every
big-band part the owner programmed is unusable. After one CC11 message, velocity stops having any effect.

This failure is different from the earlier rounds. Round 1 covered the dynamic-CRT crash, unpersisted held valves and CI
decay. Round 2 covered public artifacts, brightness × *ff* crackle, the missing durable binary and the token conflict.
Here every earlier mitigation is assumed to have been carried out perfectly.

**Timeline (simulated)**
1. **Implementation (month −1 to 0).**
   - S-008/S-010/S-011 implement D-007 and D-010 as written. An attack happens only "on note-on from silence"
     (DECISIONS.md:132). "Legato note-on while sounding: no attack" (DECISIONS.md:143). A legato note "never re-attacks"
     (DECISIONS.md:180).
   - The plan defines "sounding" in exactly one place, D-013 (DECISIONS.md:226), and the frozen header repeats it
     (`core/include/tpt/TrumpetVoice.h`, `UiState::sounding`): "a note is held, **or the meter is above −60 dBFS**". So
     the implementer reads it literally. A note-on that arrives while the previous note's release tail is still above
     −60 dBFS counts as legato.
   - With the 40 ms release ramp (DECISIONS.md:144) and modal T60s of 267/220/183 ms for partials 2–4, the tail stays
     above −60 dBFS for roughly 150–250 ms after a note-off at mf/ff.
   - A same-pitch legato note-on is a "lip slur" to the same partial (D-010), which changes nothing.
   - Every frozen test passes:
     - T-016 tests overlapping slurs;
     - T-019 waits 0.6 s after each release;
     - T-012…T-034 play single held notes;
     - T-023's random performance checks only finiteness.
     No test plays two notes separated by a short gap, or the same pitch twice.
2. **Gates.**
   - At G-005 the demos are single notes, slurs, legato phrases and 0.3 s gaps (GATES.md:61). The 0.3 s gaps are longer
     than the tail. No demo has repeated or detached notes, and the owner answers `proceed`.
   - At G-006, item 2 is "a MIDI item with notes written F♯3–C6 plays at the right pitches" (GATES.md:71). The owner
     plays a slow scale, which passes.
3. **Months 1–3.**
   - The owner imports a big-band lead part from his notation program: eighth notes at 160 bpm (187 ms), 90 % note
     length, many repeated notes and accented hits.
     - Every gap (19 ms) falls inside the tail. All repeated notes merge into one sustained tone with a shallow dip.
     - Every detached phrase is played as a lip or valve slur, with no tonguing.
     - Velocity (accents) is ignored after the first note of each phrase.
   - The exported item also starts with CC11 = 127, a common notation default. D-014 maps CC11 to breath
     (DECISIONS.md:260). Once any breath value has been seen, it "replaces velocity … until reset()" (DECISIONS.md:149;
     the `setBreath` header contract). The whole part therefore plays at L = 1 (*ff*, k_sus = 5) whatever the dynamics.
   - Items without CC11 that are played later in the session stay at *ff* until the plugin is re-prepared. Whether a
     render matches live playback depends on what was played before.
4. **Month 3: attempted workarounds.**
   - Shortening the notes to 50 % still leaves gaps (94 ms) inside the tail. Only gaps of ≥ 250 ms re-attack, which is
     impossible at tempo.
   - There is no legato parameter, no CC68 handling and no "retrigger" mode (D-016; T-026 freezes exactly 11
     parameters).
   - Alternating notes between two plugin instances on two tracks works, but it doubles the editing and breaks the real
     slurs.
5. **Month 5.**
   - A client project is due. The owner records the parts with a sample library instead and opens an issue: "no
     articulation".
   - A fix touches D-007/D-010 sound code, so D-026 R-017 applies: re-run T-029, re-issue G-005, a new G-002 for a new
     RC sha (D-025 approval scope), then G-006. That is the implementer's flow, and the implementer no longer exists.
   - Routine CI uploads no binaries (D-025). `MAINTENANCE.md` (D-026 R-019, DECISIONS.md:476-484) says how to keep CI
     alive but not how to get a new binary. The owner would have to discover the `[g002-approved rc …]` marker in
     `ci.yml` or install Visual Studio 2026 and JUCE.
6. **Discovery and impact.**
   - The plugin's core musical function, tongued articulation, never worked at tempo. Neither gates nor tests could see
     it, because every check plays isolated or overlapping notes.
   - Dynamics silently depend on whether a CC11 event was ever received.
   - The plugin is shelved after 5 months of programming time spent on it.

**How it was discovered:** by the owner in production. No frozen test, demo or checklist item plays a repeated or
detached note.

**Root causes (§3):**
- R-029: the re-articulation case (a note-on while no note is held but the tail still sounds, and same-pitch repeats) is
  undefined, and the only definition of "sounding" makes it legato.
- R-030: CC11 latches breath control for the whole session.
- R-026 (residual): the owner has no documented path to a new binary once routine CI stopped uploading any.

---

## 2. Lenses

### 2.1 Technical correctness
- **Re-articulation is undefined (R-029, new).**
  - D-007 covers a note-on "from silence", and D-010 covers a note-on "while sounding". A note-on while **no note is
    held** but the release tail still rings is the normal case for tongued or detached playing, and it is assigned
    to neither cleanly.
  - Read literally with D-013 (DECISIONS.md:226), it is legato. With the "a note is held" reading it is an attack, but
    D-007's attack starts from "lips at y = y0_frac·h0; pressure rises from 0". That implies a reset in the middle of a
    ringing resonator, and the plan does not say whether the mode states are kept.
  - Same-pitch repeats are not covered at all:
    - a same-pitch note-on while sounding → lip slur to the same partial → no audible event;
    - overlapping same-pitch notes in the 16-note stack (DECISIONS.md:186) → the first note-off can end the second note.
  - No frozen test exercises any of this (§1). Every interpretation passes.
- **Warp ring sizing still counts only one direction (R-028, insufficient).**
  - f7c98ab added "min(|p⁺|, 30 kPa) (sign kept)" (DECISIONS.md:211). Offsets can therefore reach ±119 samples at
    f_int = 96 kHz and ±238 at 192 kHz.
  - A causal warp needs a fixed base delay D ≥ the largest advance, and a look-behind of D plus the largest retard
    (negative p⁺).
  - The plan clamps the advance at 250 inside a 256-sample ring (DECISIONS.md:210-212). That leaves 6 samples for the
    retard side.
  - My spike shows that the retard side is the **larger** one. Details in §3.
- **Checked, no new issue:**
  - the `rc-artifacts` ancestor check and marker regex (DECISIONS.md:429-440);
  - `unsupportedRate()` ownership (S-013/S-014);
  - the read-only `--long-hold` / `--brightness-ff` modes (DECISIONS.md:497-500);
  - nlohmann_json linked PRIVATE into `TrumpetVST` for `heldValves`.
- **Minor inconsistency (Low, in R-032).** D-003 still says the held valves are "reset by `reset()`"
  (DECISIONS.md:34), while D-026 R-014 says `reset()` keeps them.

### 2.2 Dependency and supply-chain drift
- Nothing new beyond rounds 1–2. The pins are unchanged and SHA-checked. `pluginval` is hash-verified. The per-platform
  wheel hashes are the round-2 Low note.
- The TinySOL cache path skips md5 verification when the marker exists (S-015 step 2). A cache can only be written from
  the same ref (pull-request caches are scoped to the PR ref), so this is not a poisoning path. No entry.

### 2.3 Invalid research assumptions
- **"Legato always" goes beyond the intake.**
  - A-010 says "Monophonic, last-note priority; velocity → attack (tonguing strength)". A-007(b) defines legato as
    slurring between notes.
  - D-014 adds "legato (D-010)" as the mode, and D-010's trigger "while sounding" turns every closely spaced note into a
    slur.
  - No claim or spike covers note sequences. Every lip-model spike (`regime_map.py`, `onset_grid.py`, `timbre_spike.py`,
    `overblow_spike.py`) renders one note from silence, or one slur. The attack evidence (C-089) is for onsets from
    silence only (R-029).
- **CC11 as breath.**
  - A-010 maps "breath CC2/CC11 → dynamics once received". In General MIDI and in most notation exports, CC11 is
    *expression* (a volume trim, usually 127). Nothing checks how the sticky latch (`TrumpetVoice.h`: "until reset()")
    behaves with real MIDI material (R-030).
- **D-012 worst case was reasoned, not run (R-028).** The f7c98ab formula ("≈ 238 samples < 250") is a one-sided
  estimate. Lesson #0 applies again: produce the value by running the reference implementation.

### 2.4 Implementer misinterpretation
- **R-029.**
  - "Sounding" has one written definition, the meter-based one, and the implementer will apply it to D-007/D-010.
  - The other reading ("a note is held") leaves the attack mechanics during a ringing tail unspecified (reset the lips?
    keep the modes?). The implementer has to guess, and no test checks the guess.
- **R-028.**
  - The base delay is not specified: no value and no rule for reporting it. The implementer must choose between:
    - reporting it in `latencySamples()`, which breaks frozen T-025 (`lat <= 64`, and `lat == 0` at ≥ 88.2 kHz,
      `tests/core/test_voice_robustness.cpp:41-43`);
    - leaving it unreported, which no document states.
  - The register's R-028 mitigation says "unreported delay ≤ 1.3 ms documented". It is not documented anywhere in `plan/`:
    `grep -i delay plan/*.md` finds only the decimator stage delays (DECISIONS.md:119).

### 2.5 Integrity (Rule 9)
- **The register overstates R-028.** "unreported delay ≤ 1.3 ms documented (T-025 ≤ 64)" (RISK_REGISTER.md:35) is not in
  the plan. This is the same pattern round 2 found for R-012/R-013.
- **C-100 provenance is misattributed** (claims.json, C-100):
  - The claim is sourced to the R5 adversarial review ("version 2026-10-08", "accessed 2026-10-08"). The visibility check
    was actually made by pre-mortem round 2 on 2026-10-14 and re-checked on 2026-10-15, as the claim text itself says.
  - Its `kind` is "numerical", and its tags (`domain:musical-acoustics`, `domain:brass`, `subject:github-actions-pins`)
    do not describe a repository-visibility fact.
  - No data is falsified, but a reader cannot trace the check to the step that made it (R-032, Low).
- **ATTRIBUTION.md "Changes" is inaccurate for the G-005 bundle.**
  - It says the TinySOL recordings "are never committed or distributed".
  - The G-005 bundle commits 12 TinySOL excerpts, trimmed to 1.0 s, normalised to −23 dBFS RMS and faded (GATES.md:61),
    to the public `evidence/G-005` branch. Its `ATTRIBUTION.txt` is a copy of that section.
  - CC BY 4.0 §3(a)(1)(B) requires indicating modifications. This is a residual of R-025 (§4).
- **No fabricated numbers found.** The pass-7 items match the f7c98ab diff. The S-011 and S-009 logs are tool-generated
  by design.

### 2.6 Past lessons (each lesson in `lessons/`, with the concrete item that applies it)
| # | Lesson | Applied by | Verdict (round 3) |
|---|---|---|---|
| 0 | L-20261008T151500Z reasoned expected value | `t028_values.py`, `expected_values.py`, T-032, S-004 | Applied for frozen values. **Regressed in D-012:** the f7c98ab worst case (238 < 250) was reasoned, and it omits the larger negative side (R-028). |
| 1 | L-20261008T010500Z circular FFT | C-089, T-028 slower-attack check, S-004/S-005/S-015 | Applied. |
| 2 | L-20261007T150300Z hand-typed constant | `expected_values.py --check`, T-032, S-003 `gen_fingering_table.py` | Applied. |
| 3 | L-20261007T150400Z threshold vs reference spread | `ref_spread.py`, T-029 docstring | Applied for T-029. A future re-articulation test (R-029) must be spiked before freezing. |
| 4 | L-20261007T150500Z linear-bore ceiling | `timbre_spike.py`, `dyn_spike.py`, D-012 | Applied. |
| 5 | L-20261007T150600Z control mappings over their range | T-034, S-011 `--brightness-ff`, DR-RANGE | Applied for the knobs. **Not applied to velocity → attack in sequences** (only from silence, C-089) or to the CC11 → breath latch (T-019 checks the level only): R-029, R-030. |
| 6 | L-20261007T150700Z regime per note | `regime_map.py`, T-012, S-009 long hold | Applied. |
| 7 | L-20261007T150000Z no pip/sudo | ENVIRONMENT no-sudo variant, S-000 | Applied. Reconfirmed: this sandbox has no numpy, so my spike used pure Python. |
| 8 | L-20261007T150100Z credentials | HANDOFF, S-000 step 1, DR-CRED, S-KNOW scan | Applied. Scan clean. |
| 9 | L-20261007T150800Z host accessors | D-013, `setHeldValves`, `releaseOverblowControl`, `unsupportedRate()` (f7c98ab) | Applied. |
| 10 | L-20261007T150200Z restart = fresh state | HANDOFF:107 | Applied, but it conflicts with DR-CRED "continue local work" (R-031). |
| 11 | L-20261007T150900Z sha256sum backslash | `run_pluginval.sh` | Applied. |
| 12 | L-20261007T151000Z evidence off branch | D-019 orphan branches | Applied. |
| 13 | L-20261015T090000Z public-repo artifacts | D-025, G-002 trigger, S-002 Done when, S-000 visibility | Applied. **Missing from the `PRIOR_KNOWLEDGE.md` checklist:** it has 13 rows (0–12), and it is "the list the cold read and the pre-mortem work from" (R-032). |

### 2.7 Scale and performance
- **Big-band use:** 4–6 mono instances at RTF ≤ 0.05 each use 20–30 % of one core. That is fine.
- **R-029 workaround cost:** the only workaround (two alternating instances per part) doubles the instance count and
  the editing.
- **R-028:** a ring large enough for both directions (≈ 512 samples at 192 kHz) costs memory only.

### 2.8 Security
- **Secrets:** no tokens in the tree or the history.
- **DR-CRED and S-000:** these remove the R-027 conflict as far as the plan can (§4).
- **`rc-artifacts` trigger:** it needs push access plus the marker, so only the owner or implementer can fire it. No
  new exposure.
- **Fork PRs:** they run with read-only tokens, and no `pull_request_target` is used.
- No new security risk.

### 2.9 Operational realities
- **G-006 checklist (GATES.md:71)** has no articulation item (repeated or detached notes at tempo) and no item for an
  imported MIDI file with CC11/CC1 lanes. Notation exports are the owner's realistic input (R-029, R-030).
- **Restart versus DR-CRED (R-031).**
  - DR-CRED (DECISIONS.md:530) says "continue local work … resume pushing once `gh auth status` succeeds".
  - The gate protocol ends sessions while waiting (GATES.md:15-17), and a restart uses "only the remote repository"
    (HANDOFF.md:107).
  - Local commits made during a 401 period are therefore lost at the next session boundary.
  - The `BLOCKED.md` from DR-CRED cannot be pushed either. The human only sees the session message.
- **Owner rebuild path (R-026, residual).**
  - Since D-025 (round 2), routine CI uploads no binaries.
  - `MAINTENANCE.md`'s contents (DECISIONS.md:476-484) still cover only keeping CI alive. They do not say how the owner
    triggers `rc-artifacts` (the marker commit and his own G-002 record) or builds on Windows.

---

## 3. Root cause analysis

Severity follows the rubric in the request. Likelihood is L, M or H.

### R-029: Re-articulation is undefined. Notes that follow within the release tail, and same-pitch repeats, are played legato, so the trumpet cannot tongue at tempo
- **Lens:** technical correctness / implementer misinterpretation / invalid research assumption. **Severity: High.**
  - Tongued articulation is a core feature of the instrument. With the only written definition of "sounding", it fails
    for every gap shorter than the release tail.
  - There is no workaround inside the plugin: there is no legato or retrigger control, and T-026 freezes the 11
    parameters.
  - The implementer is forced to guess the behaviour, because D-007 and D-010 leave the "no note held, tail ringing"
    case undefined, and no frozen test covers it.
  - It is not Critical: nothing is corrupted, and the defect is audible.
- **Likelihood: M.**
  - The header's `UiState::sounding` comment and DECISIONS.md:226 both define "sounding" by the −60 dBFS meter. That is
    the reading an implementer who follows the text will use.
  - Even with the alternative reading, same-pitch overlaps and attack-during-tail mechanics remain unspecified.
- **Evidence:**
  - DECISIONS.md:132 (attack only "from silence"), :143 and :180 (no attack "while sounding"), :144 (40 ms release; lips
    and resonator keep running), :226 ("sounding" = held or meter > −60 dBFS) and :186 (stack without a duplicate rule).
  - Modal T60 of the open state (`table_draft.json`): 267 / 220 / 183 ms for modes 2 / 3 / 4.
    - From an mf/ff level of −10…−20 dBFS to −60 dBFS is 40–50 dB, which takes about 0.67–0.83 × T60, plus the ramp.
    - That gives about 150–250 ms of "sounding" after a note-off.
    - An eighth note at 120 bpm lasts 250 ms; at 90 % length the gap is 25 ms.
  - Tests:
    - T-016 overlaps notes (`test_voice_overblow.cpp:90-91`);
    - T-019 re-strikes only after 0.6 s of silence (`test_voice_midi_ui.cpp:28-39`);
    - T-017 waits 0.4 s between notes (`test_voice_overblow.cpp:149-150`).
  - No spike renders a note sequence. C-089 covers onsets from silence only.
- **Root cause:**
  - The planner equated "mono legato" (D-014) with "every note-on during sound is a slur". A-010 says only "monophonic,
    last-note priority; velocity → attack (tonguing strength)", and A-007(b) limits legato to slurs.
  - The only state machine written down is silence → attack → sustain → release → idle. There is no tongue-stop or
    re-attack transition.
- **Traces to:** A-007, A-010, D-007, D-010, D-013, D-014, C-089, T-016, T-019, G-005 (demos), G-006 item 2, SC-6.
- **Proposed mitigation:**
  1. **D-007/D-010: decide legato by held notes, not by the meter.**
     - (a) Note-on while **another** note is held (overlap) → legato, as D-010 now.
     - (b) Note-on while **no** note is held, including during the release tail → **re-articulation**: pm falls from its
       current value to 0 over 4 ms (raised cosine; `kTongueStopS` in `VoiceTuning.h`), then the D-007 rise from 0
       with the new velocity's L. Lip and mode states are **kept**: no reset, so no click.
     - (c) Note-on of the **same** pitch while it is held → re-articulation as in (b). The note stack keeps one entry
       per pitch with a count, so the first note-off of two overlapping same-pitch notes does not end the second.
  2. **Optional:** CC68 (legato footswitch) ≥ 64 makes every note-on legato. This is additive, MIDI only, and not a new
     parameter.
  3. **Spike before freezing** (lessons #0 and #3):
     - Port the (b) schedule into `research/spikes/lipsim.cpp` (pm schedule only).
     - Render concert 65 as four eighth notes at 160 bpm at 90 % and at 50 % length, mf and ff, and four detached notes
       of an open-horn arpeggio.
     - Measure the 5 ms RMS dip between notes and the re-attack rise time.
  4. **New frozen test T-035 (amendment, red-verified, thresholds from the spike):**
     - four onsets detected in the repeated-note render, each re-attack reaching −6 dB of the steady level within 60 ms;
     - an overlapping same-pitch pair keeps sounding after the first note-off;
     - T-016 still passes (overlap = slur).
  5. **Gates:**
     - G-005 demo 10 `repeated-and-detached` (the T-035 renders plus a short big-band-style phrase).
     - G-006 checklist item 8: "an eighth-note passage at 160 bpm with repeated notes articulates every note; overlapping
       notes slur".

### R-030: CC11 (GM expression) latches breath control for the session: velocity is ignored after one CC11 message, and renders depend on playback history
- **Lens:** invalid research assumption / operational. **Severity: Medium.**
  - Dynamics are silently wrong, but there is a workaround: delete the CC11 events, or send CC121, which resets breath
    (DECISIONS.md:266).
  - The behaviour follows a documented intake default (A-010).
- **Likelihood: M.** Notation exports and General MIDI files commonly carry CC11 = 127 at the start of an item.
- **Root cause:**
  - D-014 maps CC11 → `setBreath(v/127)` (DECISIONS.md:260).
  - `setBreath`'s contract is "replaces velocity … until reset()" (`TrumpetVoice.h`; DECISIONS.md:149).
  - Nothing clears the latch at transport start. `reset()` runs only when the host re-prepares the plugin. So
    CC11 = 127 means L = 1 (*ff*) for every later note, in every item.
  - T-019 checks only that breath sets the level (`test_voice_midi_ui.cpp:73-88`).
- **Traces to:** A-010, D-008, D-014, T-019, G-006.
- **Proposed mitigation:**
  1. Ask the human at G-005 (one added question) whether CC11 should be breath or expression.
     - If expression: CC11 scales the output gain (−∞…0 dB, 127 = 0 dB), and only CC2 latches breath.
  2. In either case, the processor calls `setBreath(-1)` on a host transport start (playhead `isPlaying` false → true;
     additive and logged). Renders then do not depend on earlier playback. REAPER resends the item's CC state as the
     item plays.
  3. Document CC121 and the latch in the README.
  4. Add to the G-006 checklist: "an item starting with CC11 = 127 still follows velocity (or the documented choice)".

### R-031: DR-CRED tells the implementer to continue local work while pushes fail, but sessions end at gates and restarts use only the remote, so that work is lost
- **Lens:** operational / implementer misinterpretation (introduced by the round-2 R-027 mitigation). **Severity:
  Medium.**
  - Work is redone, and the steps are idempotent (PLAN.md:17).
  - No product data is affected. The cost is rework and a confusing log: `EXECUTION_LOG.md` entries for unpushed
    attempts vanish.
- **Likelihood: L–M.** The expected 401 is exactly the G-004 revocation case that DR-CRED names. At that point the
  implementer is waiting on a gate and ends the session (GATES.md:15-17).
- **Root cause:**
  - DR-CRED (DECISIONS.md:530): "continue local work, and resume pushing once `gh auth status` succeeds".
  - HANDOFF.md:107: a restart uses "only the remote repository".
  - The `BLOCKED.md` that DR-CRED asks for cannot reach the remote either.
- **Traces to:** DR-CRED, R-027, HANDOFF halt protocol, GATES waiting rule, L-20261007T150200Z.
- **Proposed mitigation:** amend DR-CRED.
  - "Do not start a new step while pushes fail. Finish and commit the current step, then state the credential request
    and the list of unpushed commits (`git log origin/<branch>..HEAD --oneline`) in the session message.
  - Do not end the session with unpushed commits unless the human says so.
  - After a restart, re-run the unpushed steps (idempotent) and log the loss in `DEVIATIONS.md`."

### R-032: Planning-record accuracy: lesson checklist, C-100 provenance, D-003 text
- **Lens:** past lessons / integrity. **Severity: Low.** **Likelihood: H** (all three are present now).
- **Root cause:**
  - `research/PRIOR_KNOWLEDGE.md` lists 13 lessons (rows 0–12). L-20261015T090000Z, the 14th, is not in the checklist
    that the cold read, the pre-mortem and S-RETRO work from (PLAN.md S-RETRO inputs).
  - C-100 cites the R5 review (2026-10-08) for a check made by pre-mortem round 2 (2026-10-14/15). Its `kind` and
    tags are wrong (§2.5).
  - D-003 says held valves are "reset by `reset()`" (DECISIONS.md:34), which contradicts D-026 R-014.
- **Traces to:** L-20261015T090000Z, C-100, D-003, D-026.
- **Proposed mitigation:**
  - Add row 13 (High, "check repository visibility before any binary upload") and its Application row (D-025, G-002,
    S-000, S-002).
  - Re-source C-100 to `premortem/round-2.md` (accessed 2026-10-14) plus the 2026-10-15 re-check: `kind` "factual",
    tags `tech:github-actions`, `subject:github-repository-visibility`.
  - Regenerate the K-item with `build_claims.py`.
  - Change D-003 to "default open; kept by `prepare()`/`reset()` (D-026 R-014)".

### Insufficient earlier mitigations (re-raised under their IDs)

**R-028 (Medium, likelihood M): the ring sizing ignores the negative (retard) side, and the base delay is unspecified.**
- **Spike** (method in the preamble; brightness 1, L_nl = 1.7 m):

  | Written, dynamic | p⁺ max / min (kPa) | advance / retard at f_int 96 kHz | advance / retard at 192 kHz | span at 192 kHz |
  |---|---|---|---|---|
  | 60 mf | +1.3 / −3.6 | 5 / 14 | 10 / 29 | 39 |
  | 69 ff | +5.8 / −11.8 | 23 / 47 | 46 / 94 | 140 |
  | 78 ff | +9.2 / −13.7 | 36 / 54 | 73 / 109 | 182 |
  | 81 ff | +13.1 / −16.1 | 52 / 64 | 105 / 128 | 233 |
  | 84 ff | +12.3 / −17.1 | 49 / 68 | 97 / 136 | 234 |

  Advance and retard are in samples.
- **What the plan states:**
  - D-012 bounds |p⁺| at 30 kPa in both directions (DECISIONS.md:211). The design bound is therefore ±238 samples at
    192 kHz, a 476-sample span.
  - It also clamps the advance at 250 in a 256-sample ring (DECISIONS.md:210-212).
  - A causal warp outputs at a fixed delay D ≥ the advance bound and must keep D + the retard bound in the ring.
  - With D = 250, only 6 samples remain for the retard. Even written 60 mf at 96 kHz (14 samples) exceeds that.
- **Possible outcomes:**
  - The retard side is clamped silently. That is an asymmetric warp and a different timbre from the spike, and the
    S-011 counter may count only advance clamps.
  - It wraps, which gives round 2's clicks.
  - The implementer reports D in `latencySamples()`, and frozen T-025 fails: at 48 kHz, 125 + 16 > 64, and the test
    requires 0 at ≥ 88.2 kHz.
- **Register overstates the mitigation:** its "unreported delay documented" is not in the plan (§2.5).
- **Mitigation:**
  1. D-012: base delay D = ceil(β·L_nl,max·30 kPa/(ρc³)·f_int) + 2 samples (allocated at `prepare()`), and a ring of
     2·D + 4 samples (≈ 480 at 192 kHz, 244 at 96 kHz). The offset is clamped to [−D, +D] on **both** sides, and the
     counter counts both.
  2. D-006 and the README: D is an unreported fixed delay (≤ 1.3 ms at f_int = 192 kHz, ≤ 1.4 ms at 88.2 kHz), so T-025
     stays as frozen.
  3. S-011 `--brightness-ff` logs the advance and retard maxima separately.
  4. Correct the register text.

**R-025 (residual Medium): the RC artifacts' licence content is incomplete, and the build id is not in the binary.**
- **AGPLv3 text and source pointer.** The D-025 artifact contains `LICENSE` (Apache-2.0), `NOTICE`,
  `THIRD_PARTY_NOTICES.md` and `ATTRIBUTION.md` (DECISIONS.md:434-440).
  - The binary is an AGPLv3 work (C-098), but the AGPLv3 text is not included.
  - `BUILD_INFO.txt` names the sha but not the repository URL. AGPLv3 §4 and §6(d) require the licence text and clear
    directions to the Corresponding Source.
- **MIT notices.** The MIT licences of the VST3 SDK and nlohmann/json (both compiled into the binary) require their
  permission notice text. D-021's table gives only name, licence and URL.
- **Build id.** `TPT_GIT_SHA` in the UI was proposed in round 2 and not adopted. Once the VST3 is installed, it cannot
  be matched to its `BUILD_INFO.txt`.
- **TinySOL modifications.** ATTRIBUTION.md's "Changes" line says the recordings are never committed. The G-005 bundle
  publishes modified excerpts (§2.5).
- **Mitigation:**
  - Add `COPYING-AGPLv3.txt` (from the JUCE checkout), the MIT texts of the VST3 SDK and nlohmann/json, and the
    repository URL plus "source: <url>/tree/<sha>" in `BUILD_INFO.txt` to the D-025 staging list.
  - Add a `TPT_GIT_SHA` compile definition shown in the editor caption area (additive; it does not affect T-033).
  - Add to the ATTRIBUTION "Changes" line: "gate bundles contain 1.0 s excerpts, RMS-normalised to −23 dBFS, with
    10 ms fades".

**R-026 (residual Medium): no documented owner path to a new binary.**
- Since routine CI uploads nothing (D-025), the round-2 instruction "push an empty commit to get a fresh build" no
  longer yields a binary.
- **Mitigation:** S-018 `MAINTENANCE.md` adds:
  1. "to get an installable build: commit your own `GATE-G-002.RESPONSE.md` (`approve`, sha), push
     `git commit --allow-empty -m "[g002-approved rc <sha7>]"`, download the `trumpet-rc-<sha7>-windows-2025` artifact
     within 90 days";
  2. the local Windows build commands (`plan/ENVIRONMENT.md`, VS 2026).

### Did the round-2 mitigations introduce a new Critical or High risk?
- **No Critical or High risk was introduced.**
- **R-027's DR-CRED** introduced R-031 (Medium).
- **R-025's D-025** introduced the R-026 residual: routine CI no longer produces binaries, and MAINTENANCE.md still
  assumes it does (Medium).
- **R-028's f7c98ab bound** made the sizing inconsistent (Medium, above).
- **S-011 `--brightness-ff` and demo 09:** sound, read-only.

---

## 4. Tables

### Proposed register entries
| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-029 | Re-articulation undefined: a note-on during the release tail (no note held) or a same-pitch repeat is played legato, so repeated and detached notes do not articulate at tempo; overlapping same-pitch notes may cut each other off | technical / misinterpretation / research assumption | High | M | D-007 attacks only "from silence"; D-010 makes every note-on "while sounding" legato; the only definition of "sounding" (D-013, `UiState`) includes the −60 dBFS tail (≈ 150–250 ms); no spike, test, demo or G-006 item plays note sequences | A-007, A-010, D-007, D-010, D-013, D-014, C-089, T-016, T-019, G-005, G-006, SC-6 | Legato decided by held notes; re-articulation (4 ms tongue stop, D-007 rise, lip and mode states kept); same-pitch repeat re-articulates; counted stack; optional CC68; spike, then frozen T-035; G-005 demo 10; G-006 item 8 | open |
| R-030 | CC11 (GM expression) latches breath for the session: after one CC11 = 127, velocity is ignored (L = 1, ff) for every later note until re-prepare; renders depend on playback history | research assumption / operational | Medium | M | D-014 maps CC11 to `setBreath`; the latch lasts "until reset()"; nothing clears it at transport start; T-019 checks level only | A-010, D-008, D-014, T-019, G-006 | G-005 question: CC11 as breath or expression; `setBreath(-1)` on transport start (additive); README (CC121); G-006 checklist item | open |
| R-031 | DR-CRED's "continue local work" while pushes fail is lost at the next session boundary (gates end sessions; restart uses only the remote); `BLOCKED.md` cannot be pushed | operational / misinterpretation | Medium | L–M | DR-CRED (round-2 R-027 mitigation) conflicts with HANDOFF restart rule and GATES waiting rule | DR-CRED, R-027, HANDOFF.md:107, GATES.md:15-17, L-20261007T150200Z | No new step while pushes fail; report unpushed commits in the session message; do not end the session with unpushed commits; re-run and log after restart | open |
| R-032 | Planning records: lesson L-20261015T090000Z missing from the PRIOR_KNOWLEDGE checklist; C-100 sourced to R5 (2026-10-08) instead of pre-mortem round 2, wrong `kind` and tags; D-003 "reset by reset()" contradicts D-026 | past lessons / integrity | Low | H | Round-2 updates not propagated to the checklist, claim metadata and D-003 | L-20261015T090000Z, C-100, D-003, D-026 | Add checklist row 13 and its Application row; re-source and re-tag C-100, regenerate the K-item; fix D-003 | open |

Corrections to existing entries:
- **R-028:** severity stays Medium, and the status should be "partly mitigated".
  - Add the residual: the ring and clamp count only advances, while the retard side is larger (spike: 136 vs 97
    samples at 192 kHz, written 84 *ff*). The base delay is unspecified.
  - The Mitigation column claims "unreported delay ≤ 1.3 ms documented", which is not in the plan.
  - New mitigation: D = ceil(advance bound) + 2; ring 2·D + 4; clamp both sides; D documented as unreported (T-025
    unchanged); S-011 logs both sides.
- **R-025:** the status should be "mostly mitigated". Add the residual (Medium):
  - the RC artifact lacks the AGPLv3 text, the MIT notice texts and a source URL;
  - there is no build id in the binary (`TPT_GIT_SHA`);
  - the ATTRIBUTION "Changes" line is wrong for the G-005 excerpts.
- **R-026:** add the residual (Medium): MAINTENANCE.md has no owner path to a new binary (`rc-artifacts` marker recipe,
  or a local Windows build), now that routine CI uploads none.

### Round-2 mitigation verdicts
| Risk | Verdict | Why |
|---|---|---|
| R-025 public repository / G-002 | **mostly sufficient** | Removed: unapproved publication. Routine CI uploads no binaries (D-025, S-002 Done when). The `rc-artifacts` job needs a marker plus `approve`, checks ancestry and covers one sha per approval. S-000 records visibility. The stub artifacts were deleted. Residual (Medium): the artifact's licence content is incomplete (no AGPLv3 text, no MIT notice texts, no source URL), there is no build id in the binary, and the TinySOL "Changes" line is inaccurate. No new Critical/High. |
| R-026 durable binary / schedule | **partly sufficient** | The 90-day RC, "download and keep" and "no reliance on the schedule" remove the false-canary assumption. But the R-025 mitigation (no routine binary uploads) removed the round-2 rebuild recipe. MAINTENANCE.md does not tell the owner how to get a new binary, so the incident's month-5 fix path still fails. |
| R-027 implementer token vs revoked tokens | **sufficient** (residual Low) | S-000 now states the token rule as a harness statement and records name/expiry if given. The human is told to revoke all three pasted tokens with no exemption, so "keep one live" has no basis. DR-CRED turns a revocation that hits the implementer into a planned halt rather than a guess. The implementer still cannot verify token identity; no plan text can. DR-CRED's "continue local work" introduced R-031 (Medium). |
| R-028 warp ring | **insufficient** | The 256-sample ring, clamp and counter, S-011 `--brightness-ff` at 48/192 kHz and demo 09 address round 2's overflow-by-advance case. The f7c98ab bound is two-sided (±30 kPa), but the sizing counts only the advance. My spike shows the retard side is larger, and with a 250 clamp only 6 samples remain for it. The base delay is unspecified, and reporting it breaks frozen T-025. The register claims documentation that does not exist. Medium; it is detectable by S-011 only if the counter covers both sides. |

COUNTS: Critical 0, High 1, Medium 5, Low 1
