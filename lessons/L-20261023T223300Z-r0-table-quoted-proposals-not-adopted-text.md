---
id: L-20261023T223300Z-r0-table-quoted-proposals-not-adopted-text
title: The R0 application table quoted the triage proposals, not the plan text that was adopted
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [applies:all, phase:research, kind:records-drift]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-23T22:33:00Z
---
## Trigger
Prior-knowledge triage is delegated to agents that return, per lesson, a proposed amendment, and a generator renders
those proposals into the record the implementer reads (`research/PRIOR_KNOWLEDGE.md`), while the planner later edits the
plan with different wording, paths or strength.

## What went wrong
The "applied as" column showed the agents' proposals, truncated. Several contradicted the adopted plan: a field the
proposal called "required" was made informative to keep a frozen fixture valid, and new tests sat under `tests/…_extra`
(inside the frozen tree) where the plan put them under `extras/`. The pre-mortem (R-043) and a cold read found it; an
implementer following a row would have broken a frozen test.

## Correction
The table says that it quotes proposals and that the plan wins on any difference; the rows that differed now point to the
adopted text, and two lessons are labelled "partly applied" with the reason.

## Prevention rule
After applying triage proposals, render the record from where each was applied (file and section), not from the
proposal; label any proposal that was not adopted in full as "partly applied" with the reason, and grep the record for
paths that do not exist in the plan.

## Detection check
Every path named in an "applied"/"amended" row of `research/PRIOR_KNOWLEDGE.md` appears in `plan/` or `HANDOFF.md`
(`grep -oE '`[a-z_./-]+/[a-z_./-]+`'` and look each up).

## Evidence
premortem/round-5.md R-043; research/coldread/pass-11.md item 13; research/spikes/build_prior_knowledge.py OVERRIDES.
