---
id: L-20261007T151000Z-large-evidence-off-impl-branch
title: Large gate evidence (WAV bundles) bloats the implementation branch
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Low
tags: [tech:github-actions, domain:audio, phase:planning, kind:process]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:10:00Z
---
## Trigger
A gate needs tens of MB of rendered audio or screenshots.
## What went wrong
chinese-strings rendered 86 MB of WAVs for one gate and had to move them to a separate `evidence/` branch mid-run.
## Correction
(backfilled) `.gitignore renders/**/*.wav`; evidence pushed to an orphan evidence branch.
## Prevention rule
Plan audio gate evidence on a dedicated orphan branch (`evidence/<gate>`), git-ignore renders on the implementation branch, and keep only SHA256SUMS on it.
## Detection check
`git ls-tree -r impl/... | grep -c '\.wav$'` is 0.
## Evidence
qualitycoding/chinese-strings impl/v0 DEVIATIONS.md (S-008), GATE-G-003a.md.
