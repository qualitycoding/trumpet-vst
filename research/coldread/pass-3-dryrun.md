# Pass 3 dry run (cold read, planning protocol 3.6)

Scope: HANDOFF reading order, S-000 .. S-018, S-RETRO, S-KNOW, read against the frozen tests, headers, stubs, tools
stubs and spikes. Read-only checks run on the current tree: `bash tests/scripts/verify_freeze.sh` prints `freeze OK`;
`python3 research/spikes/schema_check.py` exits 0. Test-case counts match D-018 (22/19/5/1/1 C++ cases, 5 plugin cases,
Python 3 pass / 6 skip). Fixture, T-006 fixture, T-033 geometry, T-028/T-010/T-008 literals and the S-008 done-when
literal (`std::fabs(1200 * std::log2(f0 / tpt::equalTemperedHz(67))) <= 15.0`) were checked against D-004, D-005,
D-013 and the stubs and are consistent. The 10 items below remain.

## Items

1. [High] plan/DECISIONS.md:353-354 (D-020, "`realism` job conditions") and PLAN.md S-002 step 1 — the job-level
   condition `if: hashFiles('tools/render/main.cpp') != ''` is not valid GitHub Actions. `hashFiles` exists only in
   step-level contexts (`steps.if`, `with`, `env`); `jobs.<id>.if` accepts only `always/cancelled/failure/success` and
   runs before any checkout. Written as specified, GitHub rejects the whole `ci.yml` ("Unrecognized function:
   'hashFiles'"), so no job (including `freeze`) runs and the S-002 done-when ("CI `freeze` job is green") cannot be met
   without a deviation. Separately `build-linux` (D-020:345) has no guard, so until S-015 it builds a `tpt_render` target
   that does not exist, fails, and `realism` (`needs: build-linux`) never starts. Fix: guard with a first step
   `test -f tools/render/main.cpp && echo present=1 >> "$GITHUB_OUTPUT"` (id `chk`) and put
   `if: steps.chk.outputs.present == '1'` on every later step of both jobs (or step-level `hashFiles`), and say that a job
   whose steps are all skipped counts as green for S-002/S-017.

2. [Medium] plan/DECISIONS.md:339 vs :345-346 (D-020 pins and jobs) — `build-linux` "uploads [tpt_render] as an artifact
   for `realism`", but the pin list has only `actions/upload-artifact` ("Use it only for logs") and no
   `actions/download-artifact`. `realism` cannot get the binary without it, T-031 rejects any `uses:` that is not a
   40-hex pin, and the implementer would have to look the SHA up (it is only in claims.json C-021: v8.0.2,
   `9000827ccba6bdab643e8b6fd33ac0654aef8333`; note v8 pairs with upload-artifact v7). Fix: add
   `actions/download-artifact@9000827ccba6bdab643e8b6fd33ac0654aef8333` (v8.0.2) to the D-020 pin list, change "only for
   logs" to "logs and the `tpt_render` binary", and mention `chmod +x` after download (artifacts drop the executable bit).

3. [Medium] plan/DECISIONS.md:220 (D-013 "`UiState` fields come from atomics written once per block") vs
   tests/core/test_voice_overblow.cpp:152-153 (T-017): `CHECK(v->keyswitch(24)); CHECK(v->uiState().valves == 0);` with no
   `process()` between them (idle, Fixed-valves mode, previous state 1-3). Implemented literally ("once per block") the
   atomics still show 1-3 and the frozen assertion fails. Fix: amend D-013 and S-011: "`keyswitch()`, `noteOn`,
   `noteOff`, `allNotesOff`, `reset` and `setParameters` also publish the UI atomics immediately; `process()` refreshes
   `soundingPartial`/`sounding` once per block."

4. [Medium] HANDOFF.md "Running the frozen suites" (`ctest -LE 'perf|plugin'`), ENVIRONMENT.md "Build and test
   commands", PLAN.md S-001 steps 2 and 4 — `plugin/CMakeLists.txt:32` registers `add_test(NAME plugin ...)` with no
   `LABELS`; tests/CMakeLists.txt labels only `perf`. `-LE` matches labels, not names, so with the plugin built the
   "headless" run still executes `tpt_plugin_tests` (research/spikes/plan_verify_ci.md lists plugin as the 5th suite of
   `ctest -LE perf`), without `xvfb-run` on Linux. Also S-001 step 4 expects "`tpt_plugin_tests` has 1 passing case and 4
   failing" but no step produces that run (step 2 runs plain `ctest`, not under xvfb). Fix: S-001 adds
   `set_tests_properties(plugin PROPERTIES LABELS plugin)` to `plugin/CMakeLists.txt` (not frozen; list it in Outputs) and a
   step `xvfb-run -a build/plugin/tpt_plugin_tests > logs/S-001-red-plugin.txt 2>&1 || true`.

5. [Medium] plan/GATES.md (G-004 and G-005 `stop` branches: "write the final report and stop") vs PLAN.md S-RETRO
   "Depends on: S-000 … S-018" and S-018 "Depends on: S-017" — after `stop` S-016 .. S-018 never pass, so by the HANDOFF
   dependency rule S-RETRO and S-KNOW can never start and the run's lessons/knowledge are never pushed. Whether the
   closing steps run, and what "final report" means (REPORT.md of S-018?), has to be guessed. Fix: add to both `stop`
   branches: "write REPORT.md with the gate outcome, treat S-016 .. S-018 as skipped (log in EXECUTION_LOG.md), then run
   S-RETRO and S-KNOW".

6. [Low] plan/DECISIONS.md:261 (D-014, CC121 "overblow control released") vs core/include/tpt/TrumpetVoice.h — the frozen
   header has no way to release the CC16 override (`setOverblowControl` takes [-1,1] and overrides "until the parameter
   value next changes"; `reset()` also drops held notes and fixed valves). The processor (S-013) must either call
   `setParameters` with a perturbed value or add a member. Not covered by T-026. Fix: say "add the additive member
   `void releaseOverblowControl() noexcept` (log in DEVIATIONS.md) and call it for CC121".

7. [Low] plan/PLAN.md S-016 step 2 ("run `tpt_calibrate`, rebuild"), S-011 step 2 ("run `tpt_calibrate` and rebuild"), S-009
   step 3 — `tpt_calibrate` links `tpt_core`, so after a `VoiceTuning.h` edit it must be rebuilt before it runs; the
   wording, followed literally, runs the stale binary with the old constants and writes `fscale` that matches the
   previous tuning. Fix: write "build, run `tpt_calibrate`, build again (re-embeds the table), re-run the tests".

8. [Low] plan/PLAN.md:409 (S-014 "Exclusive resources: none") vs its step 4 (pushes the `[gate-evidence G-004]` marker
   commit and waits for the `gate-evidence` and `plugin` jobs) and D-020 (marker must be the head commit of the push). S-015
   may run concurrently (it depends only on S-011) and pushes too; a second push can make another commit the head, so
   the evidence job never fires. Pass 2 item 9 added exclusive resources to other steps but not S-014. Fix: S-014
   "Exclusive resources: CI" (S-015 likewise when it uses the `realism` job).

9. [Low] plan/DECISIONS.md:264-265 (D-014 "non-finite floats passed to any setter are ignored (previous value kept)") vs
   core/include/tpt/TrumpetVoice.h (`setParameters` "takes p.clamped()" and `clamped()` "non-finite floats are replaced
   by the default"). For `setParameters` with NaN `brightness` the two give 0.5 (default) vs the previous value; T-023 and
   T-011 do not distinguish them, but the implementer must pick. Fix: state "setParameters follows `clamped()` (default
   replaces non-finite); the 'previous value kept' rule applies to the scalar setters".

10. [Low] tests/python/test_resonator_tools.py `test_t030_committed_table_is_current` (rtol 1e-9 on every s and R of
    13 states x 14 modes) with PLAN.md S-005 / D-020 `python` job — the table is generated on the implementer's machine
    and regenerated on the CI runner. `scipy.optimize.least_squares` (complex modal fit, bore fit) stops at tolerance, so
    BLAS or CPU differences can move results by more than 1e-9 relative, and no step says what to do if the CI
    regeneration disagrees (the test and `generate_table` determinism are only specified on one machine). Fix: add to S-005:
    "set `ftol=xtol=gtol=1e-14`, `OMP_NUM_THREADS=OPENBLAS_NUM_THREADS=1` in `generate_table`; if the CI `python` job
    still disagrees, regenerate on the CI runner (job output) and commit that table".

TOTAL: 10 items
