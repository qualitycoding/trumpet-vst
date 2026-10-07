---
id: L-20261007T150200Z-restart-means-fresh-state
title: On "Restart", do not read earlier sessions' scratch state
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Low
tags: [applies:all, phase:intake, kind:process]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:02:00Z
---
## Trigger
The human says "Restart" (or rejects a resume attempt) while older session scratchpads or branches exist.
## What went wrong
After a "Resume" was interrupted and the human said "Restart", the agent began listing other sessions' scratchpad directories; the human rejected the call.
## Correction
Started Phase 0 from scratch using only the repository, the protocol file and the human's messages.
## Prevention rule
When told to restart, use only the remote repository and the current conversation as inputs; do not open other sessions' scratchpads.
## Detection check
No command in the session references another session's scratchpad path after the restart message.
## Evidence
Planning session 2026-10-07, start.
