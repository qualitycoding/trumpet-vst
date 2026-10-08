# Sources

Generated from `research/claims.json` (planner, 2026-10-08).

Tiers follow protocol R2. Verification: "verified" sources were read directly by an agent at the stated locator, or
reproduced by a spike. Raw notes are in `research/sources/agent-*.json`.

| # | Source | Tier | Version / date | Claims | Locators (sample) |
|---|---|---|---|---|---|
| 1 | JUCE 9.0.3 release — https://github.com/juce-framework/JUCE/releases/tag/9.0.3 | 1 | 9.0.3 | C-001, C-003 | GitHub release metadata (gh api repos/juce-framework/JUCE/releases) |
| 2 | GitHub git ref API — https://api.github.com/repos/juce-framework/JUCE/git/ref/tags/9.0.3 | 1 | 9.0.3 | C-002 | object.sha / object.type |
| 3 | JUCE 9.0.2 release — https://github.com/juce-framework/JUCE/releases/tag/9.0.2 | 1 | 9.0.2 | C-003 | release metadata |
| 4 | JUCE LICENSE.md at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/LICENSE.md | 1 | 9.0.3 | C-004 | LICENSE.md lines 1-8 ('dual-licensed under the AGPLv3 and the commercial JUCE li |
| 5 | JUCE top-level CMakeLists.txt at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/CMakeLists.txt | 1 | 9.0.3 | C-005 | CMakeLists.txt:33 |
| 6 | JUCE Linux Dependencies at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/docs/Linux%20Dependencies.md | 1 | 9.0.3 | C-006 | docs/Linux Dependencies.md (whole file) |
| 7 | Catch2 v3.16.0 — https://github.com/catchorg/Catch2/releases/tag/v3.16.0 | 1 | v3.16.0 | C-007 | releases/latest + git/ref/tags/v3.16.0 -> tag object -> object.sha |
| 8 | nlohmann/json v3.12.0 — https://github.com/nlohmann/json/releases/tag/v3.12.0 | 1 | v3.12.0 | C-008 | releases/latest + git/ref/tags/v3.12.0 -> tag object -> object.sha |
| 9 | pluginval v1.0.4 — https://github.com/Tracktion/pluginval/releases/tag/v1.0.4 | 1 | v1.0.4 | C-009 | releases/latest |
| 10 | pluginval_Linux.zip — https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Linux.zip | 1 | v1.0.4 | C-010 | downloaded with curl -L and hashed with sha256sum on 2026-10-07 |
| 11 | pluginval_macOS.zip — https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_macOS.zip | 1 | v1.0.4 | C-011 | downloaded with curl -L and hashed with sha256sum on 2026-10-07 |
| 12 | pluginval_Windows.zip — https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip | 1 | v1.0.4 | C-012 | downloaded with curl -L and hashed with sha256sum on 2026-10-07 |
| 13 | pluginval CommandLine.cpp at v1.0.4 — https://github.com/Tracktion/pluginval/blob/v1.0.4/Source/CommandLine.cpp | 1 | v1.0.4 | C-013, C-014, C-015 | CommandLine.cpp:147 (getStrictnessLevel), help text ~line 363-366; CommandLine.cpp:165 (getTimeout), help text ~line 367-370; CommandLine.cpp:96-98 CommandLineV |
| 14 | pluginval README at v1.0.4 — https://github.com/Tracktion/pluginval/blob/v1.0.4/README.md | 1 | v1.0.4 | C-013 | 'Running in Headless Mode' section |
| 15 | Adding pluginval to CI at v1.0.4 — https://github.com/Tracktion/pluginval/blob/v1.0.4/docs/Adding%20pluginval%20to%20CI.md | 1 | v1.0.4 | C-015 | example blocks (macOS/Linux/Windows) |
| 16 | Bundled VST3 SDK LICENSE.txt at JUCE 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_audio_processors_headless/f | 1 | 9.0.3 | C-016 | modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt lines 1 |
| 17 | juce_audio_processors_headless.h at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_audio_processors_headless/j | 1 | 9.0.3 | C-017, C-030 | BEGIN_JUCE_MODULE_DECLARATION: 'JUCE audio processor classes without UI', depend; module declaration block |
| 18 | JUCEModuleSupport.cmake at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/extras/Build/CMake/JUCEModuleSupport.cma | 1 | 9.0.3 | C-017 | lines ~516-545: juce_vst3_headers/juce_lilv_headers/juce_ara_headers linked to j |
| 19 | actions/checkout v7.0.1 — https://github.com/actions/checkout/releases/tag/v7.0.1 | 1 | v7.0.1 | C-018 | releases/latest + git/ref/tags/v7.0.1 |
| 20 | actions/setup-python v7.0.0 — https://github.com/actions/setup-python/releases/tag/v7.0.0 | 1 | v7.0.0 | C-019 | releases/latest + git/ref/tags/v7.0.0 |
| 21 | actions/upload-artifact v7.0.2 — https://github.com/actions/upload-artifact/releases/tag/v7.0.2 | 1 | v7.0.2 | C-020 | releases/latest + git/ref/tags/v7.0.2 |
| 22 | actions/download-artifact v8.0.2 — https://github.com/actions/download-artifact/releases/tag/v8.0.2 | 1 | v8.0.2 | C-021 | releases/latest + git/ref/tags/v8.0.2 |
| 23 | actions/cache v6.1.0 — https://github.com/actions/cache/releases/tag/v6.1.0 | 1 | v6.1.0 | C-022 | releases/latest + git/ref/tags/v6.1.0 |
| 24 | actions/runner-images README — https://github.com/actions/runner-images/blob/main/README.md | 1 | main @ 2026-10-07 | C-023, C-024, C-025 | Available Images table, Ubuntu 24.04 row; Available Images table, macOS 15 Arm64 row; Windows Server 2025 row |
| 25 | [Windows] windows-latest and windows-2025 will use VS 2026 image — https://github.com/actions/runner-images/issues/14017 | 1 | 2026-05-07 | C-025 | Breaking changes / Target date (June 8-15, 2026) |
| 26 | macOS 15 arm64 image readme — https://github.com/actions/runner-images/blob/main/images/macos/macos-15-arm64-Readme.md | 1 | 20260907.0337.1 | C-026 | full file grep for auval/audio |
| 27 | clarinet-vst REPORT (sibling project CI on JUCE 9.0.3) — https://github.com/qualitycoding/clarinet-vst/blob/impl/clarinet-v1/REPORT.md | 2 | ef14e98 | C-026, C-029 | SC-1 row: CI plugin job green on Linux (xvfb), macOS (auval PASS), Windows |
| 28 | juce_AudioProcessor.h at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_audio_processors_headless/p | 1 | 9.0.3 | C-027 | path in repo tree |
| 29 | juce_audio_processors.h at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_audio_processors/juce_audio | 1 | 9.0.3 | C-027, C-030 | dependencies: juce_gui_extra, juce_audio_processors_headless; module declaration: dependencies juce_gui_extra, juce_audio_processors_headless |
| 30 | juce_Initialisation.h at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_events/messages/juce_Initia | 1 | 9.0.3 | C-028 | class ScopedJuceInitialiser_GUI doc comment ~lines 60-92 |
| 31 | juce_MessageManager.cpp at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/modules/juce_events/messages/juce_Messag | 1 | 9.0.3 | C-028 | lines 531-550 |
| 32 | JUCE .github/workflows at 9.0.3 — https://github.com/juce-framework/JUCE/tree/9.0.3/.github/workflows | 1 | 9.0.3 | C-029 | grep -i xvfb over all workflow files: no matches |
| 33 | JUCE CMake API.md at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/docs/CMake%20API.md | 1 | 9.0.3 | C-031 | juce_add_plugin section ~lines 245-300; `ICON_COMPOSER_BUNDLE` ~line 380; `LV2_P |
| 34 | JUCE CMake API.md at 8.0.12 (for diff) — https://github.com/juce-framework/JUCE/blob/8.0.12/docs/CMake%20API.md | 1 | 8.0.12 | C-031 | diff baseline |
| 35 | JUCE BREAKING_CHANGES.md at 9.0.3 — https://github.com/juce-framework/JUCE/blob/9.0.3/BREAKING_CHANGES.md | 1 | 9.0.3 | C-032 | Version 9.0.0 section, lines ~195-330 |
| 36 | How well can linear stability analysis predict the behaviour of an out — https://hal.science/hal-01245846v4 | 1 | 2017 (HAL author ver | C-033, C-034, C-039, C-042 | Sec. 2.1, eqs. (1)-(8), pp. 4-6 of HAL version; Sec. 3.1 pp. 10-11, Fig. 4; Sec. 4.1 Table 5 p. 21; Sec. 4.1 Table 5 p. 21, text pp. 21-22, Conclusions p. 26-27 |
| 37 | Numerical continuation of a physical model of brass instruments: Appli — https://hal.science/hal-03094997 | 1 | 2020 (HAL author ver | C-035, C-040, C-041, C-050 | Sec. II.A eqs. (1)-(6) pp. 4-5; eq. (15) p. 10; Sec. IV.A.1-2 and Fig. 6, pp. 10; Sec. IV.A.1, Fig. 6 and Table 1 (p. 14); p. 10 lines 177-178; Sec. IV.A.2 Figs |
| 38 | Parameter identification of a physical model of brass instruments by c — https://hal.science/hal-03837108 | 1 | 2022 (CC BY 4.0) | C-035, C-044, C-050, C-054 | Table 1 (poles s_k and residues C_k, 11 modes), pp. 4-5 of HAL version; Fig. 1; Table 1, p. 4; Table 1, p. 4 (computed); Table 2 (same values Ql=3, mu_l=2 kg/m^ |
| 39 | Inverse problem to estimate lips parameters values of outward-striking — https://hal.science/hal-04019734 | 1 | 2023 (HAL author ver | C-036, C-053 | Table I and Fig. 4, Sec. II.C, p. 8; Table II (Sec. V.C, p. 30 of HAL version); model eqs. (1)-(4) pp. 11-13 |
| 40 | Time-domain numerical modeling of brass instruments including nonlinea — https://arxiv.org/abs/1511.04247 | 2 | arXiv v1 2015 prepri | C-037, C-048 | Introduction refs [8]-[10] (bibliographic data); Sec. 3.1 eqs. (43)-(49); Table 2 (Sec. 4.2) |
| 41 | A Physical Model of the Trombone Using Dynamic Grids for Finite-Differ — https://dafx2020.mdw.ac.at/proceedings/papers/DAFx20in21_paper_25.pdf | 2 | 2021 | C-038 | Table 2 |
| 42 | An algorithm for a valved brass instrument synthesis environment using — https://www.dafx.de/paper-archive/2015/DAFx-15_submission_55.pdf | 2 | 2015 | C-038 | Sec. 2.4, eqs. (10a)-(10e) |
| 43 | A comparison of a one-dimensional finite element method and the transf — https://inria.hal.science/hal-01963674 | 1 | 2019 (HAL v2) | C-043 | Sec. 5 p. 7 lines 348-356, Fig. 3 (top left) |
| 44 | OpenWInD repository, examples/Tournemenne-Chabassier_ACTA2019/Tr_co_MP — https://gitlab.inria.fr/openwind/openwind/-/raw/master/examples/Tournemenne-Chabassier_ACT | 1 | master branch, Copyr | C-043 | file contents |
| 45 | Differences between brass instruments arising from variations in brass — https://hal.science/hal-00475561 | 2 | 2007 | C-045, C-046, C-047 | Fig. 1 caption, p. 3 (3.64 m for 350 Hz / 5 kPa / r = 6 mm, consistent with form; eq. (8) definition B = z(L)/L(ecl), L(ecl) = c/2f1; Table 1 trombone B = 0.81  |
| 46 | A simulation tool for brassiness studies (Gilbert, Menguy, Campbell),  — https://hal.science/hal-00207999 | 1 | 2008 (HAL author ver | C-045 | eqs. (1)-(2) generalized Burgers twin equations with volume (Gamma) and wall (T/ |
| 47 | Shock waves in trombones (Hirschberg, Gilbert, Msallam, Wijnands), JAS — https://hal.science/hal-01105563 | 1 | 1996 (HAL preprint d | C-046 | eq. (1), p. 4 of HAL scan (JASA p. 1755-1756) |
| 48 | Brassiness and the Understanding of Brass Instruments (Campbell, Gilbe — https://hal.science/hal-03234054 | 2 | 2020 | C-047, C-055 | Sec. 3, p. 3171 (normalising factor L_ecN; trumpet vs trombone example; narrow b; eqs. (4)-(5) p. 3171; Sec. 5 p. 3173 (B = 0.58, Dmin 8.7 mm; Kessler B = 0.74; |
| 49 | New Algorithm for Nonlinear Propagation of a sound Wave, Application t — https://hal.science/hal-01105576v1 | 1 | 2000 | C-048 | HAL notice only, no file |
| 50 | Brass instrument (lip reed) acoustics: an introduction (Wolfe, UNSW Mu — https://newt.phys.unsw.edu.au/jw/brassacoustics.html | 3 | web page, accessed 2 | C-049, C-051 | section 'Valves and slides' (2nd valve 5.9% length per semitone example; valve s; sections 'The effect of the bell', 'Weakness of the high harmonics', 'Frequenc |
| 51 | The nonlinear physics of musical instruments (N. H. Fletcher), Rep. Pr — https://www.phys.unsw.edu.au/music/people/publications/Fletcher1999a.pdf | 1 | 1999 | C-051, C-052 | Fig. 13 caption and text, p. 745 (citing Fletcher & Tarnopolsky 1999); Sec. 7, p. 744-745 (text before Fig. 13) |
| 52 | Blowing pressure, power, and spectrum in trumpet playing (Fletcher, Ta — https://pubs.aip.org/asa/jasa/article/105/2/874/559058/Blowing-pressure-power-and-spectrum | 1 | 1999 | C-052 | abstract (via search snippet; full text not accessible) |
| 53 | Trumpet Tuning Tendencies Relating to the Overtone Series with Solutio — https://blog.utc.edu/erika-schafer/trumpet-tuning-tendencies-relating-to-the-overtone-seri | 4 | web page, undated | C-054 | list of overtone deviations (+2, -14, +2, -31 c) |
| 54 | computation — research/spikes/expected_values.py | 1 | 2026-10-07 | C-054, C-056, C-069 | values(): hs_dev_p*, open_partial_written_p*, ideal_sharp_* |
| 55 | The influence of bore size on brassiness potential (Myers, Pyle, Gilbe — https://viennatalk.mdw.ac.at/papers/Pap_01_85_Myers.pdf | 2 | 2010 | C-055 | Sec. 2 Figs. 2-3: Shires (bore 11.66 mm) vs Vega (11.13 mm) trumpets: narrower b |
| 56 | Weidner, B. N., Brass Techniques and Pedagogy, ch. 4 Brass Acoustics ( — https://pressbooks.palni.org/brasstechniquesandpedagogy/chapter/brass-acoustics/ | 2 | web edition, undated | C-056, C-066, C-067, C-068 | sections 'Valve acoustics' (valve roles + intonation table) and partial-tendency |
| 57 | Partials and Overtones (JMU Brass Pedagogy) — https://sites.lib.jmu.edu/brasspedagogy/2016/05/26/partials-and-overtones/ | 2 | 2016-05-26 | C-056, C-064, C-066, C-067 | trumpet valve/intonation paragraph (summary via fetch tool) |
| 58 | Trumpet - Wikipedia — https://en.wikipedia.org/wiki/Trumpet | 4 | accessed 2026-10-07 | C-056, C-067, C-070, C-071 | Range / Valves sections (via fetch-tool extraction) |
| 59 | Arban, Complete Conservatory Method for Trumpet/Cornet, ed. Hooten & M — https://www.jazzbooks.com/mm5/download/ARB-TPAE.pdf | 1 | (c)1982; edition (c) | C-057, C-058, C-059, C-060, C-063, C-067, C-070, C-071 | printed p. vi (PDF p.3) chart, read as rendered image; Introduction 'Range' and  |
| 60 | Yamaha Trumpet/Cornet/Flugelhorn/Rotary Trumpet Owner's Manual — https://ca.yamaha.com/files/download/other_assets/8/326908/trumpet_en_om_b0.pdf | 1 | (c)2013 | C-057, C-058, C-062 | printed p.14 'Fingering Chart' (PDF p.14), read as rendered image; alternates in |
| 61 | Bb Trumpet Fingering Chart, Andrew B. Spang (Trumpet Studio of David J — http://trumpet.bandzana.net/trumpet-fingering-chart.pdf | 3 | (c)1999; PDF produce | C-057, C-058, C-059, C-060, C-061, C-062, C-065 | chart pages 1-2 (PDF pp.2-3), one card per pitch G6 down to E3; standard fingeri |
| 62 | Trumpet Fingering Chart - Interactive, Every Note F#3 to C7 — https://trumpetfingeringchart.com/ | 4 | web page, 2026 acces | C-057, C-058, C-059, C-060, C-061, C-062, C-069 | table of standard/alternate fingerings and cent estimates (via fetch-tool extrac |
| 63 | B-flat Trumpet Fingering Chart: Valve Combinations (Tunable) — https://tunableapp.com/instruments/trumpet-bb/fingering/ | 4 | web page, 2026 acces | C-059 | standard fingerings table F#3 upward; I only read the entries up to E5/D#5 befor |
| 64 | Buckner, Dr. Jim: Tuning and Valve Slides (hsutrumpets.com) — http://www.hsutrumpets.com/tuning-and-valve-slides/ | 2 | undated web page | C-062, C-064, C-065, C-066 | sections 'Third Valve Slide', 'First Valve Slide', 'Choice of Slide' |
| 65 | Trumpet Fingering Chart: Every Note, With Alternates and Tuning Fixes — https://promusicvault.com/trumpet-fingering-chart/ | 4 | web page, 2026 acces | C-064 | third-slide guidance (via fetch-tool extraction) |
| 66 | Bb Trumpet Fingering Chart and Overtone Series (Bob Gillis) — https://bobgillis.wordpress.com/2012/08/24/bb-trumpet-fingering-chart-and-overtone-series/ | 4 | 2012-08-24 | C-064, C-066 | text beside chart image; chart image itself not read |
| 67 | Trumpet Basic Tuning Rules (school band method excerpt) — https://www.americanforkband.com/uploads/1/2/6/5/12657294/trumpet_tuning.pdf | 3 | undated | C-064, C-065, C-068 | printed p.114 'How to Adjust for Other Notes' items 5-8; pitch-tendency charts p |
| 68 | Trumpet Fingering Chart - Valve Combinations, Every Note — https://thewholehorn.com/fingering-charts/trumpet/ | 4 | web page, 2026 acces | C-064 | intonation guidance (via fetch-tool extraction, lossy) |
| 69 | Trumpet Intonation (tsmp.org handout) — https://tsmp.org/IMAGES/notation/322Trump.pdf | 3 | undated | C-065 | pp.1-2; only the text labels were extractable, the notated examples are images I |
| 70 | Hugill, The Orchestra: A User's Manual - Trumpets: Range — https://andrewhugill.com/OrchestraManual/trumpet_range.html | 3 | 2015 (per fetch) | C-070 | range paragraph (via fetch-tool extraction) |
| 71 | planner spike — https://zenodo.org/api/records/3685367 | 1 | 2026-10-07 | C-072 | Zenodo REST record + downloaded archive |
| 72 | planner spike — research/spikes/ref_spread.py | 1 | 2026-10-07 | C-073, C-074 | ref_spread_result.json dyn_order_*, brass_*; ref_spread_result.json summary |
| 73 | planner spike — research/spikes/lipsim.cpp | 1 | 2026-10-07 | C-075 | session run 2026-10-07 (pm=2000..10000) |
| 74 | planner spike — research/spikes/bore_fit.py | 1 | 2026-10-07 | C-076 | bore_fit_result.json |
| 75 | planner spike — research/spikes/make_table.py | 1 | 2026-10-07 | C-077, C-079 | printout of bore comparison, table_draft.json calib_*; table_draft.log |
| 76 | planner spike — research/spikes/tmm_trumpet.py | 1 | 2026-10-07 | C-078 | session comparison 2026-10-07 |
| 77 | planner spike — research/spikes/regime_map.py | 1 | 2026-10-07 | C-080, C-081, C-087 | PTH and DYN constants; regime_map_result.json; regime_map_result.json fscale, *_cal_cents |
| 78 | planner spike — research/spikes/overblow_spike.py | 1 | 2026-10-07 | C-082 | overblow_spike_result.json |
| 79 | planner spike — research/spikes/overblow_d.py | 1 | 2026-10-07 | C-083 | overblow_d_result.json |
| 80 | planner spike — research/spikes/brighten_spike.py | 1 | 2026-10-07 | C-084 | session output 2026-10-07 |
| 81 | planner spike — research/spikes/timbre_spike.py | 1 | 2026-10-07 | C-085 | timbre_spike_result.json |
| 82 | planner spike — research/spikes/dyn_spike.py | 1 | 2026-10-07 | C-086 | session output 2026-10-07 |
| 83 | planner spike — research/spikes/onset_spike.py | 1 | 2026-10-07 | C-088 | session output 2026-10-07 |
| 84 | planner spike — research/spikes/onset_grid.py | 1 | 2026-10-07 | C-089 | onset_grid_result.txt |
| 85 | planner spike — https://bootstrap.pypa.io/get-pip.py | 1 | 2026-10-07 | C-090 | Phase 0.1 diagnostic |
| 86 | R5 adversarial review (fresh-context Opus) — research/rounds/round-3-adversarial.md | 1 | 2026-10-08 | C-091, C-092, C-093, C-094, C-095, C-096, C-097, C-098, C-09 | round-3-adversarial.md section 1 (C-018..C-022 row); round-3-adversarial.md section 1 (C-044 row), defect D-5; round-3-adversarial.md section 1 (C-073 row), def |
