---
id: L-20261007T150700Z-regime-selection-per-note
title: Simplified bore model selected the wrong register for some notes
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, domain:musical-acoustics, phase:research, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:07:00Z
---
## Trigger
A self-oscillating exciter coupled to a fitted multi-mode resonator, where the sounding mode (register/partial) depends on resonance heights and exciter tuning.
## What went wrong
saxophone-vst: the simplified TMM gave peak ratios that made vented notes lock to the wrong mode and some notes jump an octave; found only during implementation by probing each note.
## Correction
(backfilled) Post-fit residue corrections per entry plus an embouchure-adjusted reed frequency.
## Prevention rule
In planning, simulate every (fingering, intended mode) pair with the chosen exciter at default settings and record which mode it locks to; any wrong lock is a decision or a risk before freezing.
## Detection check
A spike result lists every playable note with intended vs. sounding mode; mismatches are zero or each is covered by a D-### rule.
## Evidence
qualitycoding/saxophone-vst impl/saxophone-v1 DEVIATIONS.md (S-008).
