# Cold-read pass 4 dry run (findings, appended as found)

Read: HANDOFF, PROFILE, ASSUMPTIONS, COLD_READ, DECISIONS, PLAN, GATES, ENVIRONMENT, TRACEABILITY, PROTOCOL_EXTRACTS, all
headers, all frozen C++/Python/plugin tests, stubs, CMake, plan-verify.yml, spikes used by S-004/S-005. Checked: manifest and
expected_values OK, schema_check OK, T-033 vs D-013 layout numbers, T-006 vs D-004 limits, T-011 vs D-015, T-005/T-004 vs
header, fixture vs D-002/D-023, S-004 cross-check inputs vs regime_map.py/lipsim.cpp (57 rows, 88 excluded), step/test tag
coverage, CI pins vs claims. Most cross-document checks are consistent; items below are residual.

1. [Low] plan/PLAN.md:261 (S-008 Done when) and :282 (S-009 Done when) — S-008 lets the T-025 pitch assertion in "T-023 sample
   rates and latency (T-025)" stay red "until calibration, S-009", but S-009 only runs T-012/T-013/T-007, and S-010/S-011 require
   only "previously green tags" (the operational case was never green). No step is ever required to turn it green before S-017,
   so the exemption never expires and a calibration failure at 22.05/44.1/96/192 kHz would go unnoticed until CI. Fix: add to
   S-009 Done when "`tpt_operational_tests` passes completely (the S-008 exception has expired)" and run it in S-009 step 4.

2. [Low] plan/PLAN.md:453 (S-016 Depends on) vs plan/GATES.md G-004 branches — S-016 may start only when G-004 is answered
   `proceed` or `fix-drawing` is resolved; the third response `correct-chart` (amendment by the planning agent, then "continue")
   is not covered, so the implementer cannot tell whether S-016 may start after the amendment or must wait for a re-issued gate.
   Fix: write "G-004 answered `proceed`, `fix-drawing` resolved, or `correct-chart` amendment applied and re-frozen".

3. [Low] plan/DECISIONS.md:350 (`core` job) with plan/PLAN.md:489 (S-017 accepted failures) — T-021/T-022 live in the single ctest
   `integration`, so accepted failures make `ctest -LE perf` exit non-zero; in a default Actions step (bash -e, later steps
   skipped) `ctest -L perf` then never runs on that OS, so T-024b/SC-7 cannot be evidenced on three OSes while the
   accepted failures stay red. Fix: D-020 `core` job: run `ctest -L perf` as its own step with `if: ${{ !cancelled() }}`
   (and say that the job is red only because of the named accepted failures).

4. [Low] plan/DECISIONS.md:354-356 (`realism` job) — "run T-029 and summarize.py, push results.json/results.meta.json/SUMMARY.md"
   but T-029 (tests/python/test_realism_vs_tinysol.py `rows` fixture) writes results.json into a pytest tmp dir that the job
   cannot name. Fix: state that the job first runs `python -m tools.realism.compare_tinysol --tinysol reference-data/tinysol
   --render <tpt_render> --out results.json`, then summarize.py, then pytest.

5. [Low] tools/realism/tinysol.py:9-12 (download "idempotently ... verify the md5s") vs plan/DECISIONS.md:354-355 (cache path holds
   "the extracted subset" only, key = archive md5) — on a cache hit TinySOL.tar.gz is absent, so "verify md5 of the archive"
   cannot be evaluated and the natural reading is to re-download ~GB. Fix: S-015 step 2: write `<dest>/.extracted-<archive md5>`
   after extraction and treat its presence as "done" (skip download and md5).

No High or Medium items found: no frozen test is unsatisfiable by the plan, and no Done-when is uncheckable apart from the Low items above.

TOTAL: 5 items
