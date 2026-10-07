---
id: L-20261007T150400Z-threshold-not-checked-against-reference-spread
title: Realism threshold frozen without checking the reference data's own spread
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, domain:musical-acoustics, phase:specification, kind:wrong-assumption]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:04:00Z
---
## Trigger
Freezing a pass band for a metric that compares synthesis output with a set of real recordings (attack time, centroid, harmonic levels).
## What went wrong
clarinet-vst T-022b froze attack_ratio in [0.5, 2] for 90 % of notes; the TinySOL notes themselves vary 8-34x in that metric (slow swells), so no synthetic attack could pass.
## Correction
(backfilled) TEST_CHALLENGE proposed an onset-based metric or wider band; human decision required.
## Prevention rule
Before freezing any reference-comparison threshold, compute the metric on the reference data in a planning spike and show that a single plausible synthetic value (or the reference median itself) passes the frozen criterion.
## Detection check
A spike result file in `research/spikes/` reports, for every thresholded metric, the coverage achieved by the reference median; every coverage meets the frozen fraction.
## Evidence
qualitycoding/clarinet-vst impl/clarinet-v1 TEST_CHALLENGE.md (T-022b attack criterion).
