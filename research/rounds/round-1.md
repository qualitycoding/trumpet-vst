# Research round 1 (2026-10-07) — R0, R1, R2, first R6 spikes

## R0 Prior knowledge
- Knowledge store `qualitycoding/agent-knowledge` does not exist (HTTP 404) → nothing loaded from the store.
- Sibling runs (clarinet-vst, saxophone-vst, chinese-strings) have no `lessons/` or `knowledge/` files. Their
  problems were backfilled as planner lessons (A-019) and are applied as R0 input:
  `research/PRIOR_KNOWLEDGE.md`.
- Reused patterns (not claims): clarinet-vst plan structure, CMake layout, pluginval script, freeze script.

## R1 Decompose
Question tree in `research/QUESTIONS.md` (branches E engineering, A acoustics model, F fingering/UI, R reference
data).

## R2 Breadth (three agents in parallel + planner)
| Agent / author | Questions | Output | Items |
|---|---|---|---|
| Sonnet (engineering) | Q-E1–E5, E9 | `research/sources/agent-E.json` | 32 |
| Opus (acoustics; interrupted once by a rate limit, resumed) | Q-A1–A9, Q-R3 | `research/sources/agent-A.json` | 23 |
| Sonnet (fingering; interrupted once, resumed) | Q-F1–F4 | `research/sources/agent-F.json` | 16 + F-TABLE, F-ALT |
| Planner | Q-R1, Q-E6, Q-E7 | spikes | — |

Key results: JUCE 9.0.3 @ be29c81 (AGPLv3/commercial), VST3 SDK MIT (inside juce_audio_processors_headless),
Catch2 3.16.0, nlohmann/json 3.12.0, pluginval 1.0.4 (SHA-256 of the three zips computed), actions pinned to SHAs;
Velut 2017 and Fréour 2020/2022 lip-model equations; Fréour 2022 Table 1 = measured open B♭ trumpet poles/residues;
Doc et al. 2023 Table II = per-register trumpet lip parameters and Table I measured threshold pressures; fingering
chart corroborated (Arban, Yamaha, Buckner, Spang); TinySOL record verified (C-072).

## R6 (first spikes)
- `ref_spread.py`: reference metric spread and threshold feasibility (C-073, C-074).
- `lipsim.cpp`: simulator reproduces Fréour 2022 (C-075).
- Velut trombone parameters with a provisional bore barely oscillate; transient durations of seconds near threshold
  (matches Velut 2017 Fig. 6) — superseded by the trumpet parameter sets.

## New questions raised → round 2
- Q-A10 (architecture can reach thresholds?), how to get valved bore states without measured data, attack speed,
  overblow mapping, soft dynamics.
