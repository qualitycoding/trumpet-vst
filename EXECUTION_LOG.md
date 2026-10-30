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
2026-10-24T19:42:49Z | S-009 | 1 | pass | dec4ea4 | T-012, T-013, T-007, operational pass; long hold 36/36 ok; DR-REGIME round 1: kFlTrim[3]=0.98; residue phase deviation logged
2026-10-24T19:48:50Z | S-010 | 1 | pass | be44433 | T-014, T-019 (and T-012/13/18/20) pass; DR-REGIME round 2: kFlTrim[4]=0.98 re-calibrated; long hold ok
2026-10-24T22:53:44Z | S-011 | 1 | pass | 9c20956 | T-015..T-018, T-020, T-034, T-035 pass; T-021 pass, T-022 brassy 21/25 (<25) waits for S-016 (DR-REAL); brightness-ff: 0 clamp events. Final constants: kRegisterRampS 0.01, kBrightenUp 0.5, kTongueStopS 0.028, kFlTrim p3/p4 0.97, kH0Trim p9 0.85, nlp 0.4 m
2026-10-24T22:54:12Z | S-012 | 1 | pass | b36c9d7 | T-011 pass
2026-10-28T23:19:14Z | S-015 | 1 | pass | db6b7d5 | 82 reference notes; test_t029_coverage and selection pass; pp/mf thresholds already met, ff centroid band fails (0 %) -> S-016
2026-10-28T23:22:03Z | S-014 | 1 | pass | f59237d | T-033, T-026 (all) pass; G-004 evidence branch d81d071; GATE-G-004.md issued; CI run https://github.com/qualitycoding/trumpet-vst/actions/runs/40785962043
2026-10-29T11:33:29Z | G-004 | - | answered | - | proceed (no tokens-revoked: G-005 stays blocked until recorded)
2026-10-29T12:36:27Z | S-016 | 1 | pass | b2d7cf2 | 1 DR-REAL round; T-029, T-021, T-022 pass; G-005 evidence branch 82329ff (run 40873290014); GATE-G-005.md issued, tokens-revoked missing
2026-10-30T09:49:24Z | G-005 | - | partial | - | tokens-revoked recorded (user: 'The pat has been revoked', singular; three tokens were pasted); G-005 verdict still outstanding
2026-10-30T10:27:52Z | G-005 | - | partial | - | Q1 blind test: 10/12 real picked (pairs 1 and 9 wrong); verdict outstanding
2026-10-30T10:31:45Z | G-005 | - | answered | - | iterate: attack too clean; the start of each clip identifies the real sound (up to 3 more DR-REAL rounds)
2026-10-30T11:19:12Z | S-016 | iterate-1 | pass | 72e83b9 | G-005 round 2 issued: evidence/G-005 00d9ef1 (run 41046024094), new blind draw (seed 20261030); T-029/T-021/T-022 and all frozen suites pass locally
2026-10-30T11:33:38Z | G-005 | iterate-1 | partial | - | Q1 round 2: owner could tell only pairs 6 (A) and 9 (B), both right; others indistinguishable; verdict outstanding
2026-10-30T11:56:05Z | G-005 | iterate-2 | answered | - | iterate (selected in the session): pedal tone of the underblow sounds synthetic; overblow less bright/harsh than expected
