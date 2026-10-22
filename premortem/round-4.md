# Pre-mortem round 4 (planning protocol Phase 4)

- Reviewer: fresh-context pre-mortem agent (round 4). I wrote neither the plan nor rounds 1–3. The review is read-only,
  except for this file.
- Plan reviewed: `gen-20261007T145241Z-trumpet-vst-plan`. I started reading at `52267f3` (the round-3 mitigations:
  D-010 re-articulation, the C-101/C-102 spikes, frozen T-035, D-014 CC11 expression, the D-012 two-sided warp delay, the
  DR-CRED rewrite, and the D-025 licence texts and `TPT_GIT_SHA`). The review is of `257d87d`; see below.
  `bash tests/scripts/verify_freeze.sh` printed `expected values OK` and `freeze OK` on both commits.
- Inputs: `HANDOFF.md`, `plan/*.md`, `premortem/round-1.md` … `round-3.md`, `premortem/RISK_REGISTER.md` (R-001..R-032),
  `research/PRIOR_KNOWLEDGE.md`, `lessons/` (14 files), `research/claims.json`, the frozen tests in `tests/`, and the
  spikes in `research/spikes/` (in particular `tongue_spike.py`, `release_gap_spike.py`, `lipsim.cpp`).

- **Plan moved during the review.** The cold-read commit `257d87d` ("cold read after round 3 (10 items)") landed while I
  was reading. It specifies the re-attack exactly as spiked (3 ms, no accent), adds the counted note stack, DR-ARTIC, the
  `TPT_GIT_SHA_OVERRIDE`, the CC11 initial value and CC121 reset, the full staging list and the README "MIDI and latency"
  section. That commit also contains this file's first 12 lines. **This review is of `257d87d`.** I re-ran
  `verify_freeze.sh` on it: `freeze OK`. Several candidate findings from my first read of `52267f3` were already fixed by
  it, so I do not raise them.
- **Independent checks** (session scratchpad only; nothing committed):
  - **Onset timing of re-articulated notes.** I built `research/spikes/lipsim.cpp` with `g++ -O2 -std=c++20`. The driver
    is pure Python, because this sandbox has no numpy (lesson #7). I used `table_draft.json` and the
    `regime_map_result.json` fscale, all 31 normal-range standard notes, mf (2.5 × threshold), mouthpiece pressure, and
    onset = first 5 ms window ≥ steady − 6 dB after the dip.
    - From silence with a 15 ms rise, onset is at a median of 110 ms (25–285 ms).
    - Re-articulated with a 25 ms stop and a 15 ms rise, onset is at a median of 75 ms (45–115 ms) after note-on.
    - So the tongue stop does **not** delay re-articulated notes relative to first notes: the ringing bore speeds up the
      regrowth. There is no new timing risk; the spread is the known R-006 onset issue.
  - **T-035(a) recovery criterion, proxy check.** Same driver, with the spiked mechanism (3 ms re-attack, constant p_m)
    and T-035's persistence rule (every window from 0.3 s to 0.6 s after note-on ≥ steady − 3 dB).
    - Mouthpiece dip: 10.9–22.6 dB, against the required 8 dB.
    - Recovered: 29/31 notes (0.94) against the required 0.90 (28/31). Written 54 and 55 fail.
    - T-035 is satisfiable, but the margin is one note. DR-ARTIC is the right fallback.
  - **Secret and personal-data scan** of `HEAD` and `git log --all -p` (`ghp_`, `github_pat_`, `gho_`, e-mail
    addresses other than noreply ones): clean.

---

## 1. Incident report (simulation: 6 months after handover)

**Title:** The imported parts lose their slurs, and after one fade-out the trumpet goes quiet or silent in every later
playback and render, while the editor still shows it playing.

**How it differs from rounds 1–3:**
- Round 1 covered the dynamic CRT, held valves and CI decay.
- Round 2 covered public artifacts, brightness × *ff* crackle, the durable binary and the token conflict.
- Round 3 covered notes that cannot be tongued and CC11 latching breath.

Here every round-1 to round-3 mitigation, as amended by `257d87d`, was carried out perfectly. The failure comes from
what those mitigations defined, and from what they left out.

**Timeline (simulated)**
1. **Implementation (month −1 to 0).**
   - S-010/S-011 implement D-010 exactly. A note-on is legato "if and only if at least one *other* note is held"
     (DECISIONS.md:181). The counted stack (DECISIONS.md:197) and the 3 ms re-attack match the spike. T-035 and T-016
     pass.
   - S-013 implements D-014 exactly: CC11 is an output-gain multiplier v/127 that "starts at 1, CC121 restores 1, a
     transport start leaves it unchanged" (DECISIONS.md:277-278). The gain is applied in the processor after
     `voice.process()` (PLAN.md:396). The voice's level meter and `sounding` flag therefore never see it
     (DECISIONS.md:240-241).
   - S-017: the `rc-artifacts` job is written to D-025 (DECISIONS.md:447-451). Its first run after G-002 `approve` fails
     at the ancestry check. `actions/checkout` fetches only the marker commit by default (depth 1), so the approved sha
     two commits back is not in the clone.
     - The fix changes `ci.yml`, so the tree is no longer the approved one.
     - The approval scope rule (DECISIONS.md:470-471) then needs a second G-002 round-trip and a new sha. That costs
       one gate cycle (R-034) but is resolved before handover.
2. **Gates.**
   - Demo 10 (GATES.md:61) plays legato as *overlapping* notes: the next note-on at +0.4 s, the previous note-off at
     +0.41 s. The owner hears correct slurs and answers `proceed`.
   - G-006 item 8 ("overlapping notes slur") passes. Item 9 ("an item that starts with CC11 = 127 still plays soft notes
     soft, and CC11 = 64 halves the level") passes.
3. **Months 1–3.**
   - The owner imports lead parts from his notation program and draws parts in REAPER.
   - Slurred passages are exported, or drawn with REAPER's MIDI-editor legato action ("set note ends to start of next
     note"), as **abutting** notes: the note-off and the next note-on fall on the same sample.
     - REAPER delivers the note-off first. No other note is held at the note-on, so every slur is re-articulated with a
       25 ms tongue stop.
     - Lip slurs, the plugin's headline feature after overblowing, never sound in his projects. Nothing in the README
       says that slurs need overlap. The "MIDI and latency" section (PLAN.md:554-556) lists CC2, CC11, CC121,
       keyswitches, the delay and the rate range only.
   - His parts use CC11 for hairpins. The last phrase of the chart fades to CC11 = 0.
     - He stops, rewinds to bar 1 and presses play. The first item has no CC11 event before its first hairpin, so the
       gain is still 0: the trumpet is silent.
     - The editor animates valves and the sounding partial, because the meter is upstream of the CC11 gain. He suspects
       REAPER routing and spends an evening on it.
     - Removing and re-inserting the plugin "fixes" it, until the next fade-out.
   - Offline renders depend on what was played last.
     - After a fade-out, a render from the start has a silent or quiet first section.
     - After a playback that ended at CC11 = 80, the next render is 4 dB quieter until the first CC11 event.
     - Round 3's R-030 was meant to remove exactly this property ("renders then do not depend on earlier playback").
       It came back through the expression latch.
4. **Month 4.** A client mix is delivered with the first 16 bars of the trumpet missing. The client notices it, and
   the owner does not.
5. **Month 5: workarounds.**
   - He learns to add a CC11 = 127 event at the start of every item and to overlap every slur by hand. For an imported
     big-band book, that is hundreds of edits.
   - There is no legato footswitch (the optional CC68 from round 3 was not adopted) and no tie rule for simultaneous
     note-off/note-on events.
6. **Month 6.** He shelves the plugin for a sample library. He concludes that "it either tongues everything or goes
   silent".

**How it was discovered:** in production, by the owner and by a client. No test, demo or checklist item plays abutting
slurs, an item without CC11 after a fade-out, or a render after a playback.

**Root causes (§3):**
- R-033: CC11 expression latches across transport starts and renders, and is invisible to the meter and the UI.
- R-035: abutting notes (simultaneous note-off/note-on) are always tongued. This is undocumented, there is no tie rule
  and no legato switch, so notation and REAPER-legato slurs are lost.
- R-034 (implementation-time cost): the RC job's ancestry check cannot work on a depth-1 checkout, and D-025 names the
  wrong parent.

---

## 2. Lenses

### 2.1 Technical correctness
- **CC11 expression is a session-long latch (R-033, new).** D-014 (DECISIONS.md:277-278) sets the gain from the last CC11
  value and says "a transport start leaves it unchanged". `prepareToPlay` resets the voice, but nothing says it resets
  the processor-side gain (PLAN.md:395-397). So the level of every later playback and render depends on the last CC11
  value seen, and CC11 = 0 mutes the plugin until a new CC11 or CC121 arrives. The D-013 meter and `sounding` are
  computed inside the voice, upstream of the processor gain (DECISIONS.md:240-241), so the UI keeps showing a sounding
  note.
- **Abutting notes are always re-articulated, and the tie order is unspecified (R-035, new).**
  - D-010 decides legato only by "at least one *other* note is held at that moment" (DECISIONS.md:181).
  - When a note-off and the next note-on share a sample position, the result depends on the order in which the host
    delivers the two events. D-014 says only "the processor splits each block at event sample positions"
    (DECISIONS.md:286).
    - Note-off first (REAPER's usual order for abutting notes): tongued.
    - Note-on first: legato.
  - There is no rule, no test (T-035 covers only same-pitch abutting and different-pitch overlap) and no README note. So
    every slur written as abutting notes is tongued. That covers notation exports at full note length and REAPER's
    "legato" note-length action.
- **The RC ancestry check cannot run on a default checkout (R-034, new).**
  - D-025 (DECISIONS.md:449-451) extracts the approved sha from the marker message and "fails unless that commit is an
    ancestor of `HEAD`". On a push event, `actions/checkout` fetches only the event commit unless `fetch-depth` is set.
    No document sets it: `grep fetch-depth plan/ .github/` finds nothing.
  - D-025's parenthetical "the parent of the empty marker commit" is also false under the gate protocol. The chain is:
    approved sha → `GATE-G-002.md` commit (GATES.md:3-4) → `GATE-G-002.RESPONSE.md` commit (GATES.md:9-11) → marker. So
    the approved sha is at least 3 commits back, and "builds the marker commit (identical tree)" is not literally true
    either: the tree differs by the gate files.
- **Checked, no new issue:**
  - **Re-attack shape.** It now matches C-101 (DECISIONS.md:186-188). My proxy run passes T-035(a) with 29/31.
  - **Counted stack.** It resolves round 3's same-pitch overlap case (DECISIONS.md:197-199).
  - **Tongue stop and timing.** It does not delay re-articulated onsets (preamble check).
  - **Two-sided warp delay.** The D bound ≤ 1.3 ms holds: A = 1.242 ms × f_int with β 1.2, L_nl,max 1.7 m and
    ρc³ = 4.93e7.
    - Since `257d87d`, a clamp event can only come from an implementation error.
    - The 30 kPa saturation of |p⁺| itself is not counted. The round-3 spike maximum was 17.1 kPa, so no entry.
  - **`TPT_GIT_SHA_OVERRIDE`.** The footer, the artifact name and `BUILD_INFO.txt` now agree.

### 2.2 Dependency and supply-chain drift
- **No new pins.** The round-3 RC staging adds one unpinned network fetch: the AGPLv3 text from gnu.org, used when the
  JUCE tree has none (DECISIONS.md:458-459, 467-468). It is a licence text, so there is no code-execution path. No entry.
  Recommendation: commit `agpl-3.0.txt` once, with its SHA-256, under `licenses/`.
- **`actions/checkout@v7.0.1` default depth.** This is not drift, but the RC design depends on a default that it does
  not set (R-034).
- **No other change since round 3.** The pins are unchanged (verified by `verify_freeze.sh`).

### 2.3 Invalid research assumptions
- **"The held-note stack decides legato" assumes overlapping input.**
  - The planner's evidence for legato is A-007(b) and the demos, all of which overlap the notes.
  - No claim covers how real MIDI sources encode slurs. Notation exports and DAW legato actions commonly produce
    abutting notes.
  - The plan's choice is the usual mono-synth convention, which is defensible. The unverified part is that users will
    supply overlap, and nothing tells them they must (R-035).
- **"REAPER chases CC11, so a transport-start reset is unnecessary."** This is implicit in "a transport start leaves it
  unchanged".
  - Chasing restores a value only from events before the play position *in the same item*.
  - Items without an early CC11 event, playback from before the first item, and renders all inherit the stale value
    (R-033).
- **The T-035 margin is thin** (preamble: 29/31 against 28/31 required, on a mouthpiece-pressure proxy). DR-ARTIC covers
  it. Its `kReattackS` range "2–8 ms (spike range, C-101)" (DECISIONS.md:558) is not a spike range: C-101 ran only 3 ms
  (`tongue_spike.py:2,15`; R-036, Low).

### 2.4 Implementer misinterpretation
- **R-035: the processing order of same-sample events is a guess.**
  - JUCE's `MidiBuffer` iterates in insertion order. An implementer may sort, or may not.
  - A "fix" that treats near-simultaneous events as legato would contradict T-035(a) for same-pitch repeats. Without a
    rule, two reasonable implementations give opposite slur behaviour on the same MIDI.
- **R-034: the implementer will copy D-025's ancestry check verbatim.**
  - The parenthetical "(the parent of the empty marker commit)" (DECISIONS.md:449-450) invites
    `git rev-parse HEAD^`-style logic, which also fails, because the parent is the response commit.
- **CC11 gain on `prepareToPlay`.** Neither "reset" nor "keep" is stated. Either is defensible, so behaviour after a
  sample-rate change differs between implementations (part of R-033).

### 2.5 Integrity (Rule 9)
- **No fabricated or hand-typed numbers found.**
  - C-101/C-102 match `tongue_spike_result.txt` and `release_gap_spike_result.txt`.
  - The T-035 thresholds (8 dB, 12 dB, 3 dB, 90 %) are derived from those results, and its header cites them.
- **The register overstates R-032.** It says "C-100 re-sourced and re-tagged" (RISK_REGISTER.md:39). The source was
  corrected, but `kind` is still `"numerical"` and the tags still include `domain:musical-acoustics` and `domain:brass`
  (`research/claims.json`, C-100). Round 3 asked for `kind` "factual" and repository tags. This is the same overstated-
  mitigation pattern that rounds 2 and 3 found (R-036, Low).
- **DR-ARTIC labels an unspiked range as "spike range"** (§2.3; R-036).

### 2.6 Past lessons (each lesson, with the concrete D/T/S item that applies it)
| # | Lesson | Applied by | Verdict (round 4) |
|---|---|---|---|
| −1 | L-20261015T090000Z public-repo artifacts | D-025 (no routine uploads; `rc-artifacts` behind G-002), GATES.md:33, S-000 visibility check, S-017 step 4 | Applied. The RC job's ancestry check is broken (R-034), but that fails closed: nothing is published without approval. |
| 0 | L-20261008T151500Z reasoned expected value | `expected_values.py --check` (T-032); T-035 thresholds from C-101/C-102; `257d87d` aligned the D-010 re-attack with the spike | Applied. Minor recurrence: the DR-ARTIC `kReattackS` 2–8 ms is reasoned but labelled as a spike range (R-036). |
| 1 | L-20261008T010500Z circular FFT | `tongue_spike.py:34` and `release_gap_spike.py:70` zero-pad to 4N; T-028 slower-attack check | Applied. |
| 2 | L-20261007T150300Z hand-typed constant | `expected_values.py`, T-032; S-003 table generator | Applied. T-035 literals are thresholds, not computed values. |
| 3 | L-20261007T150400Z threshold vs reference spread | T-029 docstring; T-035 thresholds below the spike minima (8 < 11.7 dB; 12 < 16.9 dB; 90 % < 97 %) | Applied. My proxy shows the recovery margin is thin (94 % vs 90 %). |
| 4 | L-20261007T150500Z linear-bore ceiling | D-012 nonlinear propagation; `timbre_spike.py`; DR-REAL | Applied. |
| 5 | L-20261007T150600Z control mappings over their range | T-034; S-011 `--brightness-ff`; DR-RANGE; G-006 item 9 (CC11 at 127 and 64) | **Partly applied to CC11:** the range is checked at two points, but history (a value left by an earlier playback) is not. That is the CC11 analogue of round 3's finding (R-033). |
| 6 | L-20261007T150700Z regime per note | T-012; S-009 long hold; T-035 checks the partial after re-articulation (`test_voice_articulation.cpp:51,67`) | Applied. |
| 7 | L-20261007T150000Z no pip/sudo | ENVIRONMENT.md no-sudo variant; S-000 | Applied. Reconfirmed: no numpy here, so my spike used pure Python. |
| 8 | L-20261007T150100Z credentials | HANDOFF.md:27-28; DR-CRED; S-KNOW schema check | Applied. Scan clean. |
| 9 | L-20261007T150800Z host-facing accessors | `setHeldValves`, `releaseOverblowControl`, `unsupportedRate()` | Applied. There is no accessor for the CC11 gain, so the UI cannot show it. That is part of R-033's diagnosis problem (not a separate entry). |
| 10 | L-20261007T150200Z restart = fresh state | HANDOFF.md:107; DR-CRED (rewritten in round 3) | Applied. The conflict is resolved (R-031). |
| 11 | L-20261007T150900Z sha256sum backslash | `tests/scripts/run_pluginval.sh`; DR-PLUGINVAL | Applied. |
| 12 | L-20261007T151000Z evidence off branch | D-019 orphan `evidence/*`; D-020 WAV rule | Applied. |

### 2.7 Scale and performance
- **Cost of the round-3 changes.**
  - The re-articulation, the counted stack (16 entries) and the per-sample CC11 one-pole cost nothing measurable.
  - The two-sided warp ring is ≤ 484 doubles per instance.
- **Big-band use (4–6 instances) is unchanged from round 3.**
- **R-033/R-035 workaround cost.** It scales with the score: one CC11 event per item and one overlap edit per slur. For
  an imported book that is hundreds of edits, which is why the incident ends in abandonment rather than a workaround.

### 2.8 Security
- **Scan clean** (preamble).
- **`rc-artifacts` with `fetch-depth: 0`** (the R-034 fix) only adds history to a read-only checkout. It needs no token
  change.
- **Other paths.** The `TPT_GIT_SHA_OVERRIDE` (DECISIONS.md:461-463) comes from the job's own regex match (`[0-9a-f]{7}`). It cannot inject
  into the compile definition.
- No new security risk.

### 2.9 Operational realities
- **The owner's realistic input is imported notation and hand-drawn REAPER items.**
  - Both produce abutting slurs (R-035) and CC11 hairpins that end low (R-033).
  - The G-005 demo 10 and G-006 items 8 and 9 use only overlapping notes and items that start at CC11 = 127. So the
    acceptance tests cannot reveal either failure.
- **R-026 residual (Low).** MAINTENANCE.md path (b), "build locally on Windows with Visual Studio 2026
  (`plan/ENVIRONMENT.md` commands)" (DECISIONS.md:509-511), and D-025's owner alternative (DECISIONS.md:476-477) both
  point to `plan/ENVIRONMENT.md`. That file has no Windows commands: its setup and build blocks (ENVIRONMENT.md:31-63)
  are Linux only (apt, Ninja, xvfb). The CI `plugin` job has the real Windows commands. Path (a) needs the R-034 fix.

---

## 3. Root cause analysis

Severity follows the rubric in the request. Likelihood is L, M or H.

### R-033: CC11 expression latches across transport starts, re-prepares and renders; CC11 = 0 mutes the plugin for later playback while the UI still shows it sounding
- **Lens:** technical correctness / operational / past lessons (#5). The round-3 R-030 mitigation introduced it.
- **Severity: Medium.**
  - The output level silently depends on playback history, and a CC11 = 0 left by a fade mutes the plugin.
  - There is a workaround: a CC11 = 127 event at each item start, CC121 (which D-014 now resets), or re-inserting the
    plugin. The README section in S-018 will name CC121.
  - Nothing is corrupted or lost: the project data is intact, and a re-render after the workaround is correct.
  - It is not Critical. The defect is audible and the cause can be found from the documentation, although the UI
    hides it.
- **Likelihood: M.**
  - Hairpins as CC11 are common in notation exports and in hand-drawn REAPER lanes.
  - A part that ends in a diminuendo leaves a low value behind.
  - An item without an early CC11 event inherits that value at the next playback from an earlier position, or at the
    next render.
- **Evidence:**
  - DECISIONS.md:277-278: "it starts at 1, CC121 restores 1, a transport start leaves it unchanged".
  - PLAN.md:395-397: the processor applies the gain to the output. Nothing in S-013 resets it in `prepareToPlay`.
  - DECISIONS.md:240-241: the meter and `sounding` come from the voice's output, before the processor gain.
  - GATES.md:71, item 9, tests an item that *starts* with CC11 = 127, and CC11 = 64. It never tests a value left over from
    earlier playback.
  - round-3.md:303-305: the stated goal of R-030 was that "renders then do not depend on earlier playback".
- **Root cause:**
  - The R-030 fix removed the breath latch but kept history-dependence for the controller that replaced it.
  - The planner relied on host CC chasing to restore CC11. Chasing only covers events earlier in the same item.
  - Lesson #5 was applied to the value range (127, 64) but not to state carried over between playbacks.
- **Traces to:** R-030, A-010, D-013, D-014, S-013, S-018, G-006 item 9.
- **Proposed mitigation:**
  1. **D-014:** the processor sets the expression gain to 1 at the same point where it calls `setBreath(-1)` (play-head
     not playing → playing) and in `prepareToPlay`. REAPER's chased CC values arrive in the first block after the
     reset, so chasing still works, and a render no longer depends on what was played before.
  2. **D-013 / S-014:** when the expression gain is < 0.99, the editor draws "Expression −N dB" next to the build id.
     This is a processor-side atomic, like `unsupportedRate()`. It is not part of `UiState`.
  3. **Test (amendment, red-verified, protocol 2E.4):** a T-026 case with a fake play-head. Send CC11 = 0, stop, start,
     note-on: the RMS equals that of a fresh instance within 0.5 dB.
  4. **G-006 item 9:** add "play an item that ends with CC11 = 0, stop, then play from bar 1 an item without CC11: full
     level; render the project: same level as playback".
  5. **README "MIDI and latency":** CC11 is reset at every transport start.

### R-034: The `rc-artifacts` ancestry check cannot run on a default (depth-1) checkout, and D-025 misnames the approved commit as the marker's parent
- **Lens:** technical correctness / implementer misinterpretation / operational. **Severity: Medium.**
  - The first approved RC run fails visibly, so nothing wrong is published: the check fails closed.
  - The fix changes `ci.yml`. The marker's code then no longer equals the approved sha, and the approval scope
    (DECISIONS.md:470-471) needs a second G-002 request and response. G-006 is delayed by one gate cycle.
  - The owner's MAINTENANCE.md path (a) (DECISIONS.md:507-509) uses the same job.
- **Likelihood: H.**
  - `actions/checkout` fetches only the event commit unless `fetch-depth` is set, and the plan never sets it.
  - Under the gate protocol the approved sha is at least three commits before the marker: `GATE-G-002.md`, then the
    response, then the marker (GATES.md:3-11).
- **Evidence:**
  - DECISIONS.md:449-451: "the commit named in `GATE-G-002.md` (the parent of the empty marker commit) … fails unless
    that commit is an ancestor of `HEAD`, and builds the marker commit (identical tree)".
  - DECISIONS.md:361: the checkout pin, with no inputs.
  - PLAN.md:529-531: S-017 step 4 pushes the marker after the response.
  - `grep -rn fetch-depth plan/ .github/` finds nothing.
- **Root cause:**
  - The marker design was checked in the abstract (cold-read pass 7 item 2; round-3 §2.1, "checked, no new issue").
  - Nobody walked it through the actual commit sequence of the gate protocol, or through `actions/checkout`'s default
    clone depth.
- **Traces to:** D-020, D-025, D-026 R-019, S-017, G-002, G-006, R-025, R-026.
- **Proposed mitigation:**
  1. **D-025:** the `rc-artifacts` checkout uses `fetch-depth: 0`. The check is
     `git merge-base --is-ancestor <sha7> HEAD`, plus
     `git diff --quiet <sha7> HEAD -- . ':(exclude)GATE-*.md'`: the code is identical to the approved commit, and only
     gate files may differ. This replaces "identical tree".
  2. **D-025:** replace "(the parent of the empty marker commit)" with "(an ancestor of the marker commit; only
     `GATE-*.md` files may differ)".
  3. **S-017 step 4:** before writing `GATE-G-002.md`, push a dry-run marker `[rc-dryrun <sha7>]`. It runs the check
     and build steps with the upload step skipped (`if:` on the marker kind). Record the green run in `GATE-G-002.md`.
     An approval is then never spent on a broken job.

### R-035: Abutting notes (note-off and next note-on on the same sample) are always tongued, with no tie-order rule, legato switch or documentation, so slurs from notation exports and REAPER's legato action are lost
- **Lens:** invalid research assumption / implementer misinterpretation / operational. The round-3 R-029 mitigation
  introduced it.
- **Severity: Medium.**
  - Lip and valve slurs, a core feature, fail for the owner's most likely input.
  - There is a workaround (overlap every slur by a few milliseconds), but nothing documents it. The only hint is G-006
    item 8 ("overlapping notes slur").
  - It is not High: legato itself works and is tested (T-016, T-035 second case, demos 02/08/10), and the workaround
    needs no code change.
- **Likelihood: M.**
  - The behaviour is the common mono-synth convention, so some users will expect it.
  - But notation exports at full note length, and REAPER's "set note ends to start of next note", produce abutting
    notes. Nothing tells the owner to overlap them.
  - With the event order unspecified, two implementations can also disagree.
- **Evidence:**
  - DECISIONS.md:181-183: legato "if and only if at least one *other* note is held at that moment".
  - DECISIONS.md:286: "the processor splits each block at event sample positions", with no order within a position.
  - T-035 covers only a same-pitch abutting pair and a different-pitch overlap (`test_voice_articulation.cpp:39-40,
    76-77`).
  - Demo 10 legato uses overlap (GATES.md:61: "previous note-off at +0.41 s").
  - The README section (PLAN.md:554-556) does not mention slurs.
  - round-3.md:271-272 proposed an optional CC68 legato switch. It was not adopted (`grep CC68 plan/` finds nothing).
- **Root cause:**
  - The R-029 fix defined legato by held notes, which is correct for overlapping input.
  - It assumed that slurred MIDI overlaps, without checking how the owner's sources encode slurs.
  - It left the same-sample order, the place where the two cases meet, unspecified.
- **Traces to:** R-029, A-007, A-010, D-010, D-014, T-035, G-005 demo 10, G-005 questions, G-006 item 8, S-018 README.
- **Proposed mitigation:**
  1. **D-014 Timing:** within one sample position, the processor dispatches note-offs before note-ons, with a stable
     sort. `tpt_render` does the same. This makes abutting = re-articulated in every host and matches T-035(a).
  2. **D-014:** CC68 (legato footswitch) ≥ 64 makes a note-on legato whenever the voice is still sounding, held or not.
     It is MIDI only and additive (the voice gets `setLegatoPedal(bool) noexcept`, logged), and the T-026 parameter list
     is unchanged. Imported parts then need one CC lane, not hundreds of overlap edits.
  3. **G-005:** add question 5, "abutting notes of different pitch: tongue (default) or slur?". If the answer is slur,
     record a new `A-###` and treat a different-pitch note-on at the same sample as a note-off as legato. This does not
     conflict with T-035, whose abutting case is same-pitch.
  4. **Demo 10:** add a fourth group (the four notes abutting, once without and once with CC68). **G-006 item 8:** add
     "abutting notes are tongued; with CC68 ≥ 64 they slur".
  5. **README "MIDI and latency":** "slurs need overlapping notes or CC68; REAPER's legato action makes notes abut, so
     they are tongued".
  6. **Test (amendment, red-verified):** a T-035 case. A different-pitch abutting pair is tongued (dip ≥ 8 dB), and is
     legato with `setLegatoPedal(true)` (no window below steady − 20 dB).

### R-036: Planning records: C-100 metadata still wrong, DR-ARTIC calls an unspiked range a "spike range", register R-032 overstated
- **Lens:** integrity / past lessons (#0). **Severity: Low.** **Likelihood: H** (all are present now).
- **Evidence:**
  - `research/claims.json` C-100: `kind` "numerical"; tags `domain:musical-acoustics`, `domain:brass` (round-3 R-032
    asked for "factual" and repository tags).
  - RISK_REGISTER.md:39: "C-100 re-sourced and re-tagged".
  - DECISIONS.md:558: `kReattackS` "within 2–8 ms (spike range, C-101)". C-101 and `tongue_spike.py:2,15` used 3 ms
    only.
- **Root cause:** partial propagation of round-3 fixes; a tuning range was reasoned rather than run.
- **Traces to:** C-100, C-101, R-032, DR-ARTIC, L-20261008T151500Z.
- **Proposed mitigation:**
  - Set C-100 `kind` to "factual" and the tags to `tech:github-actions`, `subject:github-repository-visibility`, then
    regenerate the K-item (`build_claims.py`).
  - Either run `tongue_spike.py` with re-attack 2, 5 and 8 ms (add `reattack=` to its argument list) and cite the result
    in DR-ARTIC, or change DR-ARTIC to "`kReattackS` 3 ms (spiked); other values need a spike first".
  - Correct the register status.

### Insufficient earlier mitigations (re-raised under their IDs)
- **R-030 → partly sufficient.** The breath latch is fixed, but its replacement latches (R-033).
- **R-026 residual (Low).** Path (b) and D-025's owner alternative cite `plan/ENVIRONMENT.md` for a Windows build. That
  file has only Linux commands (ENVIRONMENT.md:31-63).
  - **Mitigation:** add a "Windows (owner)" block to ENVIRONMENT.md, copied from the CI `plugin` job: VS 2026 with the
    "Desktop development with C++" workload, then `cmake -S . -B build`, `cmake --build build --config Release`, and the
    VST3 path.
  - Path (a) depends on the R-034 fix.

### Did the round-3 mitigations introduce a new Critical or High risk?
**No.**

| Mitigation | Introduced risk |
|---|---|
| CC11 → expression (R-030) | R-033 (Medium) |
| Re-articulation rule (R-029) | R-035 (Medium): the abutting-note case and the missing tie order. It removes the round-3 failure mode for repeated and detached notes. |
| Two-sided warp delay (R-028) | Nothing new: ≤ 1.3 ms, T-025 unaffected, ring ≤ 484 samples. |
| DR-CRED rewrite (R-031) | Nothing new. It is consistent with HANDOFF.md:107 and the gate waiting rule. |
| RC licence texts and build id (R-025) | Nothing new. The R-034 ancestry defect predates round 3 (it came from cold-read pass 7), but it now also blocks the owner's path (a). |

---

## 4. Tables

### Proposed register entries
| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-033 | CC11 expression latches across transport starts, re-prepares and renders: a value left by an earlier fade (CC11 = 0) mutes or attenuates later playback and renders of items without an early CC11, while the meter and UI still show the note sounding | technical / operational / past lessons | Medium | M | R-030 fix kept history-dependence ("a transport start leaves it unchanged", DECISIONS.md:277-278); host CC chasing assumed sufficient; gain applied after the voice meter (DECISIONS.md:240-241); G-006 item 9 tests only items starting at 127 | R-030, A-010, D-013, D-014, S-013, S-014, S-018, G-006 | Reset expression to 1 at transport start and in `prepareToPlay`; editor shows "Expression −N dB" when < 0.99; T-026 amendment (CC11 = 0, stop, start → fresh level ± 0.5 dB); G-006 item 9 extended (stale value, render vs playback); README | open |
| R-034 | `rc-artifacts` ancestry check fails on a default depth-1 checkout; D-025 calls the approved commit the marker's parent and the tree identical, though the gate files sit between them → first approved RC run fails and costs a second G-002 cycle; the owner's MAINTENANCE path (a) inherits it | technical / misinterpretation / operational | Medium | H | No `fetch-depth` anywhere; marker design never walked through the GATES commit sequence (`GATE-G-002.md` → response → marker) | D-020, D-025, D-026 R-019, S-017, G-002, G-006, R-025, R-026 | `fetch-depth: 0`; `git merge-base --is-ancestor` plus `git diff --quiet <sha7> HEAD -- . ':(exclude)GATE-*.md'`; fix the parenthetical; S-017 dry-run marker (`[rc-dryrun <sha7>]`, no upload) before `GATE-G-002.md` | open |
| R-035 | Abutting notes (note-off and next note-on on one sample) are always tongued, with no tie-order rule, no legato switch and no documentation → slurs from notation exports and REAPER's legato action are lost; host event order can flip the result | research assumption / misinterpretation / operational | Medium | M | R-029 fix defines legato by held notes and assumes slurred MIDI overlaps; D-014 gives no order within a sample position; round-3's optional CC68 dropped; demo 10 and G-006 item 8 use overlap only | R-029, A-007, A-010, D-010, D-014, T-035, G-005, G-006, S-018 | Note-offs before note-ons at one sample position (processor and `tpt_render`); CC68 ≥ 64 → legato while sounding (`setLegatoPedal`, additive); G-005 question 5 (tongue or slur abutting different-pitch notes); demo 10 group 4; G-006 item 8 extended; README; T-035 amendment case | open |
| R-036 | Planning records: C-100 `kind`/tags still wrong although the register says re-tagged; DR-ARTIC calls `kReattackS` 2–8 ms a "spike range" though C-101 ran only 3 ms | integrity / past lessons | Low | H | Partial propagation of round-3 fixes; reasoned tuning range (lesson #0) | C-100, C-101, R-032, DR-ARTIC, L-20261008T151500Z | Fix C-100 `kind` and tags, regenerate the K-item; spike re-attack 2/5/8 ms or restrict DR-ARTIC to 3 ms; correct the R-032 status | open |

Corrections to existing entries:
- **R-030:** status "partly mitigated". The breath latch is removed, but the CC11 replacement latches (R-033).
- **R-026:** add a residual (Low). MAINTENANCE.md path (b) and D-025's owner alternative cite `plan/ENVIRONMENT.md`
  Windows commands that do not exist (ENVIRONMENT.md:31-63 are Linux only). Add a "Windows (owner)" block taken from the
  CI `plugin` job. Path (a) depends on R-034.
- **R-032:** status "mostly mitigated". The checklist row and the D-003 fix are done (D-003 in `257d87d`). C-100 is
  re-sourced but not re-typed or re-tagged (R-036).

### Round-3 mitigation verdicts
| Risk | Verdict | Why |
|---|---|---|
| R-029 re-articulation (High) | **sufficient** for its failure mode | Legato is decided by held notes (DECISIONS.md:181), not by the meter. Release-tail and same-pitch note-ons are re-articulated with a 25 ms stop and a re-attack exactly as spiked (C-101, aligned in `257d87d`). The counted stack (DECISIONS.md:197-199) keeps overlapping same-pitch notes. T-035 is frozen and red-verified, DR-ARTIC is the fallback, and demo 10 and G-006 item 8 cover it. My checks: re-articulated onsets are not later than attacks from silence (median 75 vs 110 ms after note-on), and the T-035(a) recovery proxy passes 29/31 against 28 required, so the margin is thin. The repeated and detached notes of round 3's incident now articulate. New, separate risk: the abutting-note case (R-035, Medium). |
| R-030 CC11 latch | **partly sufficient** | Velocity is no longer overridden: CC11 is an expression gain, CC2 breath is cleared at transport start, CC121 restores the gain, and G-006 item 9 checks this. But "a transport start leaves it unchanged" re-creates history-dependent renders and a mute after CC11 = 0, hidden from the meter (R-033, Medium). |
| R-031 DR-CRED vs restart | **sufficient** | No new step while pushes fail; unpushed commits are listed in the session message; re-run and log after a restart (DECISIONS.md, DR-CRED). This is consistent with HANDOFF.md:107 and the GATES waiting rule. No new risk. |
| R-032 planning records | **mostly sufficient** | The PRIOR_KNOWLEDGE row for L-20261015T090000Z and its Application row were added. D-003 was fixed in both bullets (`257d87d`). C-100's source was corrected, but `kind` and tags were not, and the register claims they were (R-036, Low). |
| R-025 residual (licence texts, build id, ATTRIBUTION) | **sufficient** | The full JUCE, AGPLv3, VST3 SDK and nlohmann texts are staged, with paths (DECISIONS.md:458-468). There is a source URL in `BUILD_INFO.txt`. `TPT_GIT_SHA_OVERRIDE` makes the footer match the approved sha. ATTRIBUTION now describes the G-005 excerpts. Note only: the AGPLv3 fallback is an unpinned gnu.org download (no entry). |
| R-026 residual (owner path to a binary) | **partly sufficient** | MAINTENANCE.md now names both paths (DECISIONS.md:507-511). Path (a) fails until R-034 is fixed. Path (b) cites Windows commands that ENVIRONMENT.md lacks (Low, correction above). |
| R-028 residual (two-sided warp) | **sufficient** | Base delay D = ceil(A) + 2, ring 2D + 4, clamp ±(D − 1) on both sides, all computed at `prepare()`. It is documented as an unreported delay of ≤ 1.3 ms (checked: A = 1.242 ms × f_int), and T-025 is unchanged. The stale "256-sample ring" sentence and the S-011 remedy were fixed in `257d87d`. A clamp now only flags an implementation error. The 30 kPa saturation is uncounted, but the spike maximum is 17.1 kPa. |

COUNTS: Critical 0, High 0, Medium 3, Low 2
