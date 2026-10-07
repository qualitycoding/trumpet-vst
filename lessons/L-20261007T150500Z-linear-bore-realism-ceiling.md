---
id: L-20261007T150500Z-linear-bore-realism-ceiling
title: Physical model architecture fixed before checking it can reach the realism thresholds
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, domain:musical-acoustics, phase:research, kind:wrong-assumption]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:05:00Z
---
## Trigger
Choosing the synthesis architecture (exciter + linear modal bore) and freezing spectral realism thresholds in the same plan.
## What went wrong
clarinet-vst: after five calibration rounds the harmonic-level error stayed 8.9-9.7 dB (target 6 dB) because the linear bore produced square-wave-like spectra at all dynamics; mf centroid only 50 % within ±25 %.
## Correction
(backfilled) Output EQ and an even-harmonic term were added late; the gap remained and was left to the human gate.
## Prevention rule
In planning, run a spike of the chosen exciter + resonator on at least 3 notes x 3 dynamics and compare its harmonic levels with the reference recordings using the frozen metric; if the spike misses by more than the threshold, change the architecture (e.g. add nonlinear propagation or radiation filtering) before freezing.
## Detection check
`research/spikes/` contains a timbre spike result against reference recordings with per-dynamic harmonic error recorded in claims.json.
## Evidence
qualitycoding/clarinet-vst impl/clarinet-v1 REPORT.md (SC-4) and DEVIATIONS.md (S-016).
