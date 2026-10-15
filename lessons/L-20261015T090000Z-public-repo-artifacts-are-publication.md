---
id: L-20261015T090000Z-public-repo-artifacts-are-publication
title: Workflow artifacts of a public repository are a distribution, not a private hand-off
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [tech:github-actions, applies:all, phase:planning, kind:wrong-assumption]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-15T09:00:00Z
---
## Trigger
A plan or CI change uploads binaries (plugins, apps, packages) with `actions/upload-artifact`, and the plan treats the
artifact as private or as "not a release".

## What went wrong
`plan-verify.yml` uploaded the stub plugin builds as artifacts in three runs (2026-10-14). D-025 declared workflow
artifacts "private to the repository", but `qualitycoding/trumpet-vst` is public (`gh api repos/... --jq .visibility`
returns `public`), so any signed-in GitHub user could download them. This was an unapproved distribution of AGPLv3
binaries that the G-002 gate should have covered. Pre-mortem round 2 (R-025) caught it.

## Correction
Deleted all nine stub artifacts and removed the upload step from `plan-verify.yml`. D-025 and G-002 were rewritten:
binary artifacts are uploaded only after G-002 approval, by a marker-triggered job, with licence notices and build
information.

## Prevention rule
Before adding any upload of built binaries, check the repository's visibility with
`gh api repos/<owner>/<repo> --jq .visibility`. For a public repository, treat the upload as publication and gate it
behind the external-action gate.

## Detection check
`grep -rn "upload-artifact" .github/workflows/` returns no binary uploads outside a job whose `if:` requires the
approval marker; the plan's gate list shows the approval.

## Evidence
CI runs 38582474550, 38627674021 and 38627991748 (artifacts deleted on 2026-10-15); pre-mortem round 2, R-025.
