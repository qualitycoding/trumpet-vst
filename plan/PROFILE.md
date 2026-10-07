# Active profiles

| Profile | Active | Justification |
|---|---|---|
| `software` | **yes** | The deliverable is a VST3/AU/Standalone instrument plugin plus offline build and analysis tools (A-001). |
| `math` | no | No mathematical statements are produced; published acoustics results (lip-valve models, bore impedance) are consumed as cited claims. |
| `computational` | no | Numerical results are not the deliverable; numerical behaviour of the synthesis is verified as software tests. |
| `publication` | no | No manuscript, preprint or archive deposit (A-001). |

Mode flags: `software.deploys = false` (no release, package publication or running service; CI builds artifacts
only, any tag/release is behind G-002). `math.exploration`: n/a.

## Not-applicable sections (produce no artifacts, impose no checks)
- Rule 7 (evidence classes), Rule 8 (result-agnostic planning): `math`/`computational` inactive. The spirit of
  Rule 8 is still applied to the realism calibration: the procedure and thresholds are frozen, the outcome is
  not presupposed (decision rule DR-REAL in `plan/DECISIONS.md`, gate G-005).
- G-001 (headline-result gate): `math`/`computational` inactive.
- Intake blocks 0.3.3, 0.3.4, 0.3.5; R2b novelty search; 2A (all); 2B.2 provenance contract; 2B.4 unknown-outcome
  tests; 2C figures; 2D manuscript.
- `plan/OPERATIONS.md`, deployment tests, operational (deployment) forks: `software.deploys = false`.
- Layout entries: `research/NOVELTY.md`, `math/**`, `figures/SPEC.md`, `manuscript/**`.

## Applicable artifacts
`HANDOFF.md`, `plan/{PROFILE,PLAN,ASSUMPTIONS,DECISIONS,GATES,ENVIRONMENT,TRACEABILITY}.md`,
`research/{QUESTIONS.md,PRIOR_KNOWLEDGE.md,claims.json,SOURCES.md,rounds/,spikes/}`, `tests/`,
`tests/FROZEN_MANIFEST.sha256`, `premortem/{round-N.md,RISK_REGISTER.md}`, `lessons/`, `knowledge/`,
`.checkpoints/state.json`.
