# Research round 4 (2026-10-08) — verification of the round-3 corrections; saturation check

| Step | Action | Result |
|---|---|---|
| R3 | Synthesis of round 3 | No unresolved contradiction. The superseding claims C-091–C-099 replace the weakened ones. |
| R4 | Depth | C-091, C-093 and C-094 rest on the R5 reviewer's 3–4 s re-runs (archived scripts) and the planner's check below. C-097 and C-098 rest on Tier-1 repository files read by the reviewer (action.yml, CHANGE_LIST, LICENSE). |
| R6 | `research/spikes/floors_cap_check.py` | Extended floors (86, 87, 89 at ≥ 2.5× threshold; 88 at ≥ 5×) sustain for 3 s. Jumps p8→p9 for written 80–84 sustain for 2.5 s. 9/9. Output: `floors_cap_check_result.txt`. |

## Saturation
This round produced:
- **New load-bearing claims:** none. The corrections were registered in round 3, and the round-4 check confirmed them without new facts.
- **Confidence downgrades:** none.
- **Unresolved contradictions:** none.

So research is **saturated after 4 rounds**, inside the 3–6 bound. Every load-bearing claim is now `corroborated` or `verified` (`research/claims.json`). Items not meeting the bar, all non-load-bearing, are carried as risks (R-003 C vs B♭ reference, R-005 p2 tendencies, R-007 high modes).
