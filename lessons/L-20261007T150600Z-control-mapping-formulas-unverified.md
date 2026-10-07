---
id: L-20261007T150600Z-control-mapping-formulas-unverified
title: Control-to-model mapping formulas in DECISIONS were not exercised in spikes
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, tech:cpp, phase:specification, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:06:00Z
---
## Trigger
DECISIONS.md specifies formulas mapping user controls (overblow, dynamics, bow force, attack time, reed hardness) to physical-model parameters.
## What went wrong
clarinet-vst D-006 overblow shelf could not raise the centroid and D-011 zeta formula moved the default away from the spike-verified point; saxophone-vst attack tau gave 150 ms attacks and the frozen reed-flow law chattered; chinese-strings bow-force formulas gave erratic pitch. Each needed a deviation.
## Correction
(backfilled) Implementers retuned constants and logged deviations.
## Prevention rule
Every formula in DECISIONS.md that maps a control to model parameters must be run in a planning spike across the control's full range, and its default output must equal the spike's verified operating point.
## Detection check
Each such D-### cites a spike file and the spike output shows the full-range sweep.
## Evidence
clarinet-vst DEVIATIONS.md (S-008, S-011, S-016); saxophone-vst DEVIATIONS.md (S-008); chinese-strings DEVIATIONS.md (S-008).
