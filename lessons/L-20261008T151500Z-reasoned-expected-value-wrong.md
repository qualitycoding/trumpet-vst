---
id: L-20261008T151500Z-reasoned-expected-value-wrong
title: An expected value "derived by reasoning" in the expected-values generator was wrong
status: active
supersedes: []
recurrence_of: L-20261007T150300Z-hand-typed-frozen-constant
occurrences: 1
severity: High
tags: [domain:audio, tech:python, phase:specification, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-08T15:15:00Z
---
## Trigger
You are freezing a test whose expected value comes from applying a metric (onset, level, frame-based estimate) to a
synthetic signal, and the generator script computes that value with a formula you derived by hand.

## What went wrong
T-028 expected `onset_ms(100 ms ramp) == 49 ± 3`, and `expected_values.py` computed it as `(0.5 − 0.01) × 100`. The
documented metric (1 ms frames of a 440 Hz carrier, frame RMS) actually gives 53 ms. An independent re-implementation
in the cold read (pass 1, item 23) caught it. The value was "script-generated" in form only; it still came from
reasoning. This repeats L-20261007T150300Z.

## Correction
Added `research/spikes/t028_values.py`, which runs the reference metric on the exact test signals. Its outputs (53.0,
102.0) are copied into `expected_values.py`, and the script asserts that they still match. Test re-frozen.

## Prevention rule
Every expected value that results from running an algorithm on an input must be produced by executing the reference
implementation on the exact test input. A formula derived by hand is never accepted, even inside the generator script.

## Detection check
For every `EXPECTED:` name, `expected_values.py` either computes it from a closed-form mathematical definition that is
the definition itself (e.g. 12-TET), or names the spike script that produced it, and that script asserts equality.

## Evidence
tests/python/test_realism_metrics.py (T-028); research/spikes/t028_values.py; cold-read pass-1 item 23.
