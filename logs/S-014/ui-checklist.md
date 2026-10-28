# S-014 UI checklist (D-013 and S-014 bullets)

| Item | Status | Where / note |
|---|---|---|
| Layout rectangles (valve caps, casings, triggers, ladder, caption, instrument box) | built | `core/src/Layout.cpp`, T-033 |
| Colours: pressed cap amber #FFB000, up grey #9AA0A6 | built | `plugin/src/TrumpetView.cpp` |
| Trigger shown: amber and drawn extended by 0.03 in x | built | `TrumpetView::paint`, shown only while the UiState carries the trigger (realism < 0.5, D-013) |
| Harmonic ladder: target rung amber outline, sounding rung amber fill, both if equal | built | 13 rungs `p1..p13` |
| Caption `captionText(state())` inside `layout.caption` | built | no separate caption label |
| Level meter (one meter, RMS of the output over 20 ms) | deviated | `UiState` has no level field and its header is shared with frozen tests; the voice keeps the meter for `sounding` and the idle rule. The view shows a "sounding" lamp instead (DEVIATIONS.md S-014) |
| Editor size 960x600, view = top 400 px | built | `PluginEditor.cpp` |
| 9 rotary knobs with APVTS attachments | built | brightness, lip stiffness, breath noise, vibrato rate, vibrato depth, tuning A4, intonation, gain, overblow |
| 2 toggles with APVTS attachments | built | alternate fingerings, fixed valves |
| Build id "Build <TPT_GIT_SHA>" (D-025) | built | bottom right, outside the T-033 rectangles |
| Unsupported-rate overlay (D-026 R-022) | built | `paintOverChildren` when `unsupportedRate()` |
| 60 Hz timer and `refreshFromProcessor()` | built | `startTimerHz(60)` |
| Drawing by `juce::Path` only, no images | built | |
