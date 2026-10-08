# Traceability

## Success criteria → tests → steps
| SC | Criterion (measurable) | Tests | Steps |
|---|---|---|---|
| SC-1 | VST3 and Standalone build on Windows, macOS and Linux. AU builds on macOS. pluginval strictness 10 passes on 3 OSes; `auval` passes on macOS. | T-026, T-027 | S-013, S-017 |
| SC-2 | For every fingering and mode, the UI state (valves, trigger, target and sounding partial, extended, fixed) is correct within one block of the voice being settled. The drawing satisfies the layout rules. The human confirms the chart and drawing (G-004). | T-020, T-026 (editor), T-033, T-003, T-005, G-004 | S-003, S-013, S-014 |
| SC-3 | Every standard note written 54–87 is within ±10 cents of 12-TET at mf at 44.1/48/96 kHz. Written 54–84 also at pp and ff (48 kHz), and every alternate at mf. | T-013, T-018 | S-009, S-011 |
| SC-4 | Realism against TinySOL meets the D-017 thresholds, and the human signs off the blind test (G-005). | T-029, T-021, T-022, G-005 | S-015, S-016 |
| SC-5 | Every fingering locks to and sustains its partial at pp/mf/ff. Overblow/Underblow jump registers per D-009 (cap at partial 9) and brighten below o = 0.5. Fixed valves play natural partials. | T-012, T-015, T-017, T-009 | S-007, S-008, S-011 |
| SC-6 | Mono legato, last-note priority. Lip slurs jump without retriggering; valve slurs settle within 150 ms. Bend ±2 st, vibrato ±25 c, breath controller. | T-016, T-019 | S-010, S-011 |
| SC-7 | No allocation on the audio path. Real-time factor ≤ 0.05 at 48 kHz/128 on CI runners. Latency reported. | T-024, T-025, T-026 | S-008, S-017 |
| SC-8 | Parameters and alternates/fixed-valves state round-trip. Garbage state is ignored. The table parser rejects malformed input. | T-011, T-026, T-006 | S-006, S-012, S-013 |
| SC-9 | Finite, bounded output for parameter extremes, all sample rates, any block size (bitwise identical), invalid input; digital silence after release; deterministic. | T-023 | S-008 |
| SC-10 | Third-party pins (40-hex SHAs), Python lock, least-privilege CI, notices and attribution present. Freeze intact; no hand-typed frozen literals. | T-031, T-032 | S-002, S-018 |

## Tests → requirements, claims, files
| Test | File | Enforces | Claims / decisions |
|---|---|---|---|
| T-001 | tests/core/test_pitch.cpp | D-001 | expected_values.py |
| T-002 | tests/core/test_valves.cpp | D-002, D-003 | C-067 |
| T-003 | tests/core/test_fingering.cpp | SC-2, D-002 | C-056–C-065 |
| T-004 | tests/core/test_fingering.cpp | D-002, D-023 | A-002, A-003 |
| T-005 | tests/core/test_fingering.cpp | SC-5, D-003 | A-007c |
| T-006 | tests/core/test_resonator_table.cpp | SC-8, D-004 | A-015, R5 D-8 |
| T-007 | tests/core/test_resonator_table.cpp | D-004, D-011 | C-044, C-076, C-077, C-079, C-099 |
| T-008 | tests/core/test_lip_model.cpp | D-005 | C-036, C-044, C-053 |
| T-009 | tests/core/test_reference_sim.cpp | SC-5, D-005, D-006 | C-044, C-075, C-091 |
| T-010 | tests/core/test_analysis.cpp | D-017 (C++ side) | — |
| T-011 | tests/core/test_state.cpp | SC-8, D-015, D-016 | A-015 |
| T-012 | tests/core/test_voice_pitch.cpp | SC-5, D-005, D-023 | C-091 |
| T-013 | tests/core/test_voice_pitch.cpp | SC-3, D-011 | C-092 |
| T-014 | tests/core/test_voice_pitch.cpp | A-006, D-011 | C-079, C-092 |
| T-015 | tests/core/test_voice_overblow.cpp | SC-5, D-009 | C-083, C-084, C-093 |
| T-016 | tests/core/test_voice_overblow.cpp | SC-6, D-010 | C-083 |
| T-017 | tests/core/test_voice_overblow.cpp | SC-5, D-003 | A-007c |
| T-018 | tests/core/test_voice_pitch.cpp | SC-3, A-007d | C-062 |
| T-019 | tests/core/test_voice_midi_ui.cpp | SC-6, D-014 | A-010 |
| T-020 | tests/core/test_voice_midi_ui.cpp | SC-2, D-013 | A-009 |
| T-021 | tests/core/test_voice_timbre.cpp | SC-4, D-008 | C-074 |
| T-022 | tests/core/test_voice_timbre.cpp | SC-4, D-012 | C-074, C-085 |
| T-023 | tests/core/test_voice_robustness.cpp | SC-9, D-006 | — |
| T-024 | tests/core/test_alloc.cpp, test_perf.cpp | SC-7, A-012 | — |
| T-025 | tests/core/test_voice_robustness.cpp | SC-7, D-006 | — |
| T-026 | tests/plugin/test_plugin.cpp | SC-1, SC-2, SC-8, D-013–D-016 | C-027–C-029 |
| T-027 | tests/scripts/run_pluginval.sh | SC-1 | C-009–C-015 |
| T-028 | tests/python/test_realism_metrics.py | D-017 | lesson L-20261008T010500Z |
| T-029 | tests/python/test_realism_vs_tinysol.py | SC-4, A-013, D-017 | C-072, C-073, C-085, C-089, C-094, C-096 |
| T-030 | tests/python/test_resonator_tools.py | D-004 | C-076, C-077, C-099 |
| T-031 | tests/python/test_supply_chain.py | SC-10, D-020, D-021 | C-001–C-022, C-097, C-098 |
| T-032 | tests/scripts/verify_freeze.sh | SC-10 | lesson L-20261007T150300Z |
| T-033 | tests/core/test_layout.cpp | SC-2, D-013 | A-009 |

Every test maps to at least one requirement, and every SC maps to at least one test.
