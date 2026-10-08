# HANDOFF — trumpet-vst v1 (implementing agent: start here)

## Purpose
Build a physically modelled B♭ trumpet instrument plugin with overblowing and a UI that shows the fingering. It is
VST3/Standalone on Windows, macOS and Linux, plus AU on macOS. Execute `plan/PLAN.md` from S-000 to S-KNOW, without
questions, except at the human gates in `plan/GATES.md`.

**Active profile:** `software` only (`software.deploys = false`); see `plan/PROFILE.md`.

## Reading order
1. This file.
2. `plan/PROFILE.md` and `plan/ASSUMPTIONS.md` (the intake answers).
3. `plan/DECISIONS.md` (design, interfaces, decision rules).
4. `plan/PLAN.md` (steps).
5. `plan/GATES.md`.
6. `plan/ENVIRONMENT.md`.
7. `plan/TRACEABILITY.md`.
8. `research/PRIOR_KNOWLEDGE.md` (lessons checklist).
9. As needed: `research/claims.json`, `research/rounds/*.md` and `research/spikes/*`. The spikes are reference
   implementations: lip simulator, transfer-matrix model, table generator, metrics.

## Environment
Follow `plan/ENVIRONMENT.md`. There is a no-sudo variant. In it the plugin builds only in CI, while the core library,
tests and Python run locally.

**Credentials:** a GitHub token with the permissions listed there, held by `gh` only. Never write it to a file, commit,
lesson or log, and never expand it into a command line.

## Running the frozen suites
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j4
ctest --test-dir build -LE perf --output-on-failure      # unit, integration, operational, alloc (+ plugin if built)
ctest --test-dir build -L perf --output-on-failure        # T-024b, Release only
xvfb-run -a build/plugin/tpt_plugin_tests                 # T-026 (Linux)
.venv/bin/python -m pytest tests/python                   # T-028..T-031 (T-029 needs TPT_RENDER and TINYSOL_DIR)
bash tests/scripts/run_pluginval.sh <Trumpet.vst3>        # T-027
```

## Freeze
`bash tests/scripts/verify_freeze.sh` must print `freeze OK` before and after every step. It checks:
- the SHA-256 of every file in `tests/FROZEN_MANIFEST.sha256` (tests, fixtures, test scripts);
- every `EXPECTED:` literal, via `research/spikes/expected_values.py --check`.

**Immutability rule:** you cannot modify, skip, mark as expected-failure, or weaken a frozen test. If you believe a
frozen test or fixture is wrong:
1. halt that path;
2. write `TEST_CHALLENGE.md` (item ID, evidence, proposed fix);
3. continue only with steps that do not depend on it.

The planning agent then runs an amendment (protocol 2E.4).

## Steps at a glance
| Step | Title | Gate |
|---|---|---|
| S-000 | Environment & access verification (creates `impl-20261007T145241Z-trumpet-vst-plan`) | — |
| S-001 | Baseline build and red run | — |
| S-002 | CI workflow and third-party notices | — |
| S-003 | Pitch, valves, fingering, note resolution | — |
| S-004 | Analysis utilities and YIN cross-check | — |
| S-005 | Resonator tools and table | — |
| S-006 | Table parsing and embedding | — |
| S-007 | Lip model and reference integrator | — |
| S-008 | Real-time voice core | — |
| S-009 | Tuning calibration | — |
| S-010 | Expression and MIDI | — |
| S-011 | Overblow, slurs, fixed valves, alternates | — |
| S-012 | State persistence | — |
| S-013 | Plugin processor | — |
| S-014 | Editor, drawing, G-004 bundle | **G-004** |
| S-015 | Offline renderer and TinySOL tooling | — |
| S-016 | Realism calibration, G-005 bundle | **G-005** |
| S-017 | Performance and plugin validation on 3 OSes | — |
| S-018 | Final verification and report | — |
| S-RETRO | Retrospective | — |
| S-KNOW | Lessons & knowledge push | **G-003** |

The plan begins with **S-000** and ends with the closing steps **S-RETRO** and **S-KNOW**. A step may start as soon as
every step in its `Depends on` list has passed. Independent steps may run concurrently unless they share an exclusive
resource.

## Human gates
- **G-004:** fingering chart and drawing.
- **G-005:** realism listening.
- **G-003:** lessons and knowledge push.
- **G-002:** any release, tag, publication or merge to `main`. No step does this; never perform such an action
  without G-002 sign-off.

While halted at a gate, continue any step that does not depend on the gate's outcome. Never take an external or
irreversible action while halted.

## Halt and deviation protocol
- **Unanticipated situation:** apply the decision rules in `plan/DECISIONS.md`; DR-DEFAULT applies when no specific
  rule fits. Every deviation is logged in `DEVIATIONS.md` (step, situation, choice, rationale).
- **Halt and write `BLOCKED.md`** when a choice would touch frozen tests, security, data integrity, a public interface
  (signature change; additive members are allowed and logged), or research integrity.
- **Execution log:** after every step attempt, append to `EXECUTION_LOG.md`:
  `UTC time | step ID | attempt | pass/fail/blocked | commit | note`.
- **Restart:** if you are told to restart, use only the remote repository and the instructions given
  (lesson L-20261007T150200Z).

## Lessons and knowledge during the run
- Lessons go to `lessons/L-<UTC>-<slug>.md` (Appendix A template; see the existing files for the format).
- Knowledge items go to `knowledge/K-<UTC>-<slug>.md` (Appendix B).
- Both are on the implementation branch, committed before continuing.
- Before every such commit, run `python3 research/spikes/schema_check.py`; it must exit 0.
- They are pushed to the knowledge store only in S-KNOW, after G-003.

## Rules restated verbatim (planning protocol v3.2)
9. **Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures. In addition:
   * `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
   * `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

11. **Lessons** `[All]`: Whenever the executing agent corrects its own execution, it records a lesson before continuing. The *executing agent* is the planning agent while it runs this protocol, and the implementing agent while it runs the plan. A *correction* is any of:
    * redoing or reverting something the agent did;
    * a command, assumption, or edit that failed and was fixed by taking a different approach;
    * a check (test, cold read, proof review, pre-mortem, CI) that caught the agent's own mistake;
    * a correction from the human.

    Retrying the same action after a transient failure is not a correction, and neither is a failure the protocol plans for (such as tests failing red against stubs).
    * Each lesson is one file, `lessons/L-<YYYYMMDDTHHMMSSZ>-<slug>.md`, on the working branch, using the template in Appendix A. It is committed before work continues.
    * A lesson carries the *identifying information* that lets a future agent recognise the situation before repeating the mistake: the trigger (the circumstances and observable signals), the mistake, the correction, a prevention rule written as one imperative, checkable instruction, and a detection check.
    * Every lesson is classified with tags from the store taxonomy (Appendix C), so lessons can be read as a group: for example, all `tech:java` lessons, or all `domain:audio` lessons.
    * If the mistake repeats a lesson loaded in R0, the new lesson names it in `recurrence_of` and strengthens its prevention rule; a recurrence means the earlier rule did not work.
    * If the same mistake recurs within the run, increase `occurrences` on the run's existing lesson and add the new evidence, instead of writing a second file.

12. **Knowledge Base** `[All]`: All supporting research and derived knowledge is recorded as knowledge items, so later work can reuse it instead of researching it again. Whenever either agent discovers a new fact about the task's Subject (its technologies, formats, APIs, platforms, tools, mathematical objects, or venues), it adds an item before continuing.
    * Each item is one file, `knowledge/K-<YYYYMMDDTHHMMSSZ>-<slug>.md`, on the working branch, using the template in Appendix B. It records the fact, its source and locator, the version it applies to, its confidence, and the claim it derives from (`C-###`), if any.
    * Every item is classified with tags from the store taxonomy (Appendix C), including at least one `subject:` tag, so it can be found again.
    * A fact that turns out wrong or outdated is never edited away: a new item `supersedes` it, so the history remains.

13. **Knowledge Store** `[All]`: Lessons and knowledge items persist across tasks in the knowledge store recorded at intake (`A-###`; layout in Appendix C). The store is read at the start of research (R0) and written only at the end of implementation, after gate `G-003` (3.7.2). During a run, new lessons and items accumulate on the working branch. Lessons and items must never contain credentials, tokens, secrets, or personal data; redact them (for example, `github_pat_***`).
