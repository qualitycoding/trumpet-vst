---
id: L-20261007T150300Z-hand-typed-frozen-constant
title: Hand-typed expected constant in a frozen test was wrong
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, tech:cpp, phase:specification, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:03:00Z
---
## Trigger
Writing frozen tests whose assertions contain numeric expected values (frequencies, cents, ratios) computed by the planner.
## What went wrong
clarinet-vst T-001 froze `equalTemperedHz(94, 442.0) == 1872.4721`; the correct value is 1873.1307508272341. The test stayed red and needed a TEST_CHALLENGE and human approval.
## Correction
(backfilled) The implementer raised TEST_CHALLENGE.md; the constant must come from the formula.
## Prevention rule
Generate every numeric expected value in a frozen test with a committed script (`research/spikes/expected_values.py`) and copy its printed output; cite the script in the test comment.
## Detection check
`python research/spikes/expected_values.py --check tests/` reports every literal it generated as matching.
## Evidence
qualitycoding/clarinet-vst impl/clarinet-v1 TEST_CHALLENGE.md (T-001), commit ef14e98.
