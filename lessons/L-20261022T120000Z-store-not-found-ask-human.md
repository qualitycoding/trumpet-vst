---
id: L-20261022T120000Z-store-not-found-ask-human
title: Knowledge store declared absent from a 404 on a guessed name; the real (private) store was missed
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [applies:all, tech:github, phase:intake, kind:wrong-assumption]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-22T12:00:00Z
---
## Trigger
At intake (0.3.7) the knowledge store location is a proposed default (for example `<owner>/agent-knowledge`), the human
does not answer that item, and a check of the default returns 404 or a repository listing does not show a likely store.
Private repositories may be invisible to the token or to `gh repo list`.

## What went wrong
The planner treated the 404 on `qualitycoding/agent-knowledge` as "store does not exist". R0 then loaded nothing, and the
whole plan was built without the 226 lessons and 273 knowledge items in the real store, `qualitycoding/knowledge` with
`qualitycoding/Lessons`. The intake repository listing showed only public repositories. The human corrected the location
after the plan was marked READY, so R0 and its consequences had to be redone.

## Correction
R0 was re-run against both store repositories (A-018 corrected, A-025), and its findings were applied to the plan before
it was re-pushed.

## Prevention rule
If the knowledge store check fails at intake, do not proceed with an empty R0. Search the owner's repositories including
private ones (`gh search repos --owner <owner> knowledge` and `lessons`, `gh api user/repos?affiliation=owner`), and if
nothing is found, ask the human for the store location in the intake batch as an explicit, non-defaulted question.

## Detection check
`.checkpoints/state.json` `knowledge_store.readable` is true, or `plan/ASSUMPTIONS.md` records the human's explicit answer
that no store exists.

## Evidence
plan/ASSUMPTIONS.md A-018 (corrected 2026-10-22); research/PRIOR_KNOWLEDGE.md (R0 re-run).
