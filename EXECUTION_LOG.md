# Execution log
UTC time | step | attempt | status | commit | note
2026-10-24T17:33:49Z | S-000 | 1 | pass | c016db1 | no-sudo variant, plugin OFF locally
2026-10-24T17:35:42Z | S-001 | 1 | pass | 10ac9ab | red: unit 22/22, integration 22/22, operational 5/5, alloc 1/1, perf 1/1; pytest 9F+2E,3 pass,6 skip; plugin not built locally
2026-10-24T18:25:31Z | S-002 | 1 | pass | 2854500 | CI run https://github.com/qualitycoding/trumpet-vst/actions/runs/40117137955: freeze green; core/plugin/python red only on stubs (D-018); dumpbin/TSan/ASan/pluginval/auval steps present
2026-10-24T18:26:04Z | S-003 | 1 | pass | f6a13d0 | T-001..T-005 pass; support selftest passes
2026-10-24T18:27:34Z | S-004 | 1 | pass | 5cec710 | T-010, T-028 pass; YIN cross-check 56/56 rows agree (1 excluded)
2026-10-24T19:17:36Z | S-005 | 1 | pass | 6ac08dd | T-030 pass (6); table sha256 e90919aac5b8681c; bore fit max 5.10 c; loops 0.0898/0.1844/0.2840 m; radiation 2 sections rms 2.61 dB
2026-10-24T19:18:35Z | S-006 | 1 | pass | 5267e38 | T-006, T-007 pass
2026-10-24T19:19:08Z | S-007 | 1 | pass | 7656cdb | T-008, T-009 pass
2026-10-24T19:28:58Z | S-008 | 1 | pass | 961a3ef | operational 4/5 cases pass (only the T-025 pitch assertion fails, needs S-009), alloc pass, noise power pass, RTF 0.0105
