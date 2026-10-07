---
id: L-20261007T150900Z-windows-sha256sum-backslash
title: GNU sha256sum prefixes the hash with a backslash for Windows paths
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Low
tags: [tech:github-actions, phase:verification, kind:tooling]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:09:00Z
---
## Trigger
A bash script on a Windows GitHub runner hashes a file whose path contains backslashes (e.g. under `RUNNER_TEMP`).
## What went wrong
clarinet-vst frozen `run_pluginval.sh` failed its checksum test on Windows: output was `\<hash>`.
## Correction
(backfilled) CI exported RUNNER_TEMP via `cygpath -m` for that step.
## Prevention rule
In frozen bash scripts, hash files via stdin (`sha256sum < file`) so the output never contains an escaped path.
## Detection check
`grep -n 'sha256sum "' tests/scripts/*.sh` finds no path-argument use.
## Evidence
clarinet-vst DEVIATIONS.md (S-017).
