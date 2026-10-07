---
id: L-20261007T150100Z-no-credentials-in-command-lines
title: Do not interpolate credentials into shell command lines
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [applies:all, tech:github-actions, phase:intake, kind:security]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:01:00Z
---
## Trigger
Wanting to inspect token scopes or call an authenticated HTTP API from the shell while a token is held by `gh` or an env var.
## What went wrong
Ran `curl -H "Authorization: Bearer $(gh auth token)" ...` to read token-scope headers; the human rejected the tool call and supplied a new token.
## Correction
Use `gh api` (which authenticates internally) for GitHub calls, and test permissions with harmless operations (`git push --dry-run`, `gh api repos/<o>/<r> --jq .permissions`).
## Prevention rule
Never expand a credential into a command line, URL, header argument or file; use the tool's own authenticated client.
## Detection check
`grep -nE 'auth token|Bearer \$|github_pat_' <session commands / scripts>` finds nothing.
## Evidence
Planning session 2026-10-07: rejected tool call during 0.1 permission checks.
