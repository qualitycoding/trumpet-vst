---
id: L-20261007T150800Z-host-facing-accessors-missing
title: Voice interface lacked accessors the plugin host layer needs
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:juce, tech:cpp, phase:specification, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:08:00Z
---
## Trigger
Freezing core headers (stubs) for a voice class that a JUCE AudioProcessor will wrap.
## What went wrong
clarinet-vst and saxophone-vst both had to add `latencySamples()` to the public voice header during implementation (D-005 required reporting latency, no accessor existed); saxophone also needed new table fields.
## Correction
(backfilled) Additive public methods, logged as deviations.
## Prevention rule
Before freezing interfaces, list every value the plugin layer must read from the core (latency, UI state snapshot, tuning, parameter ranges) and give each an accessor in the stub header.
## Detection check
Every host-facing requirement in DECISIONS.md names the header method that provides it.
## Evidence
clarinet-vst DEVIATIONS.md (S-008); saxophone-vst DEVIATIONS.md (S-007, S-008).
