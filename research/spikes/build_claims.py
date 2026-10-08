"""Builds research/claims.json from the raw R2 findings (research/sources/agent-*.json) plus the planner's spike
claims, and mechanically generates one knowledge item per claim (protocol R2 / Rule 12) into knowledge/.

Usage (from the repository root): python3 research/spikes/build_claims.py
Re-running is idempotent: claim IDs are stable (order of the source files), knowledge files are rewritten.
"""
import json, pathlib, re

ROOT = pathlib.Path(__file__).resolve().parents[2]
RUN = "gen-20261007T145241Z-trumpet-vst-plan"

KIND = {"E": "software", "A": "numerical", "F": "policy"}
TAGS = {
    "E": ["tech:juce", "tech:cmake", "tech:github-actions", "domain:audio"],
    "A": ["domain:musical-acoustics", "domain:brass", "domain:audio"],
    "F": ["domain:brass", "domain:audio"],
}
SUBJECT = {"Q-E1": "juce", "Q-E2": "cpp-dependencies", "Q-E3": "pluginval", "Q-E4": "vst3-sdk", "Q-E5": "github-actions-pins",
           "Q-E9": "juce-headless-testing", "Q-A1": "brass-lip-model", "Q-A2": "brass-regime-selection",
           "Q-A3": "brass-pedal-note", "Q-A4": "trumpet-bore-geometry", "Q-A5": "trumpet-input-impedance",
           "Q-A6": "brass-nonlinear-propagation", "Q-A7": "trumpet-radiation", "Q-A8": "trumpet-blowing-pressure",
           "Q-A9": "trumpet-intonation", "Q-R3": "trumpet-reference-recordings", "Q-F1": "trumpet-fingering",
           "Q-F2": "trumpet-fingering", "Q-F3": "trumpet-slide-triggers", "Q-F4": "trumpet-valves"}
# what each source question informs (decisions / tests); see plan/DECISIONS.md, plan/TRACEABILITY.md
INFORMS = {"Q-E1": ["D-020", "D-022", "T-031"], "Q-E2": ["D-022", "T-031"], "Q-E3": ["T-027"], "Q-E4": ["D-021", "T-031"],
           "Q-E5": ["D-020", "T-031"], "Q-E9": ["T-026"], "Q-A1": ["D-005", "T-008", "T-009"], "Q-A2": ["D-005", "D-009", "T-012"],
           "Q-A3": ["D-009", "T-015"], "Q-A4": ["D-004", "T-030"], "Q-A5": ["D-004", "T-007"], "Q-A6": ["D-012", "T-022"],
           "Q-A7": ["D-012"], "Q-A8": ["D-008", "T-008"], "Q-A9": ["D-011", "T-014"], "Q-R3": ["D-017", "T-029"],
           "Q-F1": ["D-002", "T-003"], "Q-F2": ["D-002", "T-003", "T-018"], "Q-F3": ["D-002", "D-013", "T-020"],
           "Q-F4": ["D-004", "D-011"]}
NOT_LOAD_BEARING = {"E-3", "E-9", "E-17", "E-23", "E-26", "E-29", "E-30", "E-31", "A-5", "A-6", "A-16", "A-23", "F-8", "F-15", "F-16"}

# Planner spike claims (R6). Locators point at committed spike code and result files.
SPIKES = [
    ("Q-R1", "TinySOL (Zenodo record 3685367, v6.0, CC BY 4.0) contains 96 'Trumpet in C' ordinario notes, concert MIDI 54-86 at pp/mf/ff (ff missing at 54, 85, 86), 44.1 kHz 16-bit mono WAV; archive md5 36030a7fe389da86c3419e5ee48e3b7f, metadata md5 a86c9bb115f69e61f2f25872e397fc4a; the Pitch ID column is concert MIDI.",
     "https://zenodo.org/api/records/3685367", "Zenodo REST record + downloaded archive", ["D-017", "T-029"], ["trumpet-reference-recordings"]),
    ("Q-R2", "Reference spread (TinySOL TpC, concert 54-82): onset (10 % -> 50 % of the first-300 ms maximum, 1 ms frames) q10/median/q90 = 26/61/164 ms pp, 22/41/103 ms mf, 22/62/99 ms ff; a leave-one-out neighbour prediction gives harmonic MAD (H1..H8) mean 2.6/1.6/1.5 dB and q90 4.2/2.6/2.3 dB (pp/mf/ff), centroid ratio within [0.75, 1.33] for 100/100/93 % of notes, onset ratio within [0.5, 2] for only 52/76/89 %.",
     "research/spikes/ref_spread.py", "ref_spread_result.json summary", ["D-017", "T-029"], ["trumpet-reference-recordings"]),
    ("Q-R2", "In TinySOL TpC the spectral centroid (pitch-normalised) increases strictly pp < mf < ff for 100 % of pitches 54-82, and the mean level of harmonics 6-10 (re strongest) is higher at ff than at pp by 13.7 dB (q10), 32.5 dB (median), 38.5 dB (q90).",
     "research/spikes/ref_spread.py", "ref_spread_result.json dyn_order_*, brass_*", ["D-017", "T-021", "T-022"], ["trumpet-reference-recordings"]),
    ("Q-A1", "The planner's lip simulator (research/spikes/lipsim.cpp: Freour 2022 eq. 1, bilinear modal update at 96 kHz, closed-form implicit flow) reproduces Freour et al. 2022: with their Table 1 open-trumpet modes and Table 2 lips (Ql 3, mu 2 kg/m^2, y0 0.1 mm, b 8 mm) at f_l = 382.18 Hz it oscillates at f0 = 479.7 Hz for p0 = 3 kPa (paper: ~470 Hz, Hopf ~2.2 kPa) and does not start from rest at 2.0 kPa.",
     "research/spikes/lipsim.cpp", "session run 2026-10-07 (pm=2000..10000)", ["D-005", "D-006", "T-009"], ["brass-lip-model"]),
    ("Q-A4", "A 7-parameter bore (cup, throat 1.83 mm, backbore, conical leadpipe, 0.460-inch cylinder, Bessel bell to 61.5 mm radius; own geometry, no third-party data) fitted by least squares to the Freour 2022 open-valve pole frequencies reproduces modes 2-11 within +-5.3 cents (rms 3.1 c) with air at 27 degC; total length 1.408 m; mode 1 at 84.0 Hz (measured 83.2 Hz).",
     "research/spikes/bore_fit.py", "bore_fit_result.json", ["D-004", "T-007", "T-030"], ["trumpet-bore-geometry"]),
    ("Q-A4", "The fitted-bore TMM reproduces the measured mode Q factors within ~15 % but over-predicts peak heights above ~800 Hz by a factor 1.5-3.5 (modes 7-11), so the resonator table applies per-mode measured/TMM frequency and height ratios of the open horn to every valve state.",
     "research/spikes/make_table.py", "printout of bore comparison, table_draft.json calib_*", ["D-004", "T-007"], ["trumpet-bore-geometry"]),
    ("Q-A4", "Deriving valved resonances by uniformly scaling the open-horn modes by 1/(1 + sum of loop ratios) deviates from the TMM of the same bore by up to 86 cents (mode 1) and 40-75 cents (modes 2-8 for 1-3 and 1-2-3), so uniform scaling is rejected.",
     "research/spikes/tmm_trumpet.py", "session comparison 2026-10-07", ["D-004"], ["trumpet-bore-geometry"]),
    ("Q-A9", "With valve loops tuned so each single valve lowers partials 3-6 by its nominal interval (loops 0.0896 m valve 2, 0.1841 m valve 1, 0.2837 m valve 3), the fitted-bore TMM gives partial-2 valved notes 15-60 cents flatter than the open partial 2 and 1-3 / 1-2-3 on partial 3 29-49 cents sharp, consistent with the teaching that low F#3/G3 are already low and only D4/C#4 need the third slide (C-F9).",
     "research/spikes/make_table.py", "table_draft.log", ["D-002", "D-011", "T-014"], ["trumpet-intonation"]),
    ("Q-A2", "With the Doc 2023 Table II lip set (W 12 mm, Ql 20, mu 9 kg/m^2; f_l/f_res and h0 per partial, extrapolated above partial 6) and pressure = k x threshold (k pp 1.3, mf 2.5, ff 5), every standard fingering written 54-87 locks to its intended partial at pp, mf and ff; written 88 (E6, 0, p10) speaks only at ff and written 89 (F6) not at pp.",
     "research/spikes/regime_map.py", "regime_map_result.json", ["D-005", "D-023", "T-012"], ["brass-regime-selection"]),
    ("Q-A2", "Uncalibrated, the model plays 4-117 cents above equal temperament; one per-note frequency scale (all poles, residues and f_l x s) found by secant iteration at mf keeps pp, mf and ff within +-4.6 cents of equal temperament for all standard notes 54-87 (alternate written 80 = 1-2-3 p9 fails: jumps at pp).",
     "research/spikes/regime_map.py", "regime_map_result.json fscale, *_cal_cents", ["D-011", "T-013"], ["brass-regime-selection"]),
    ("Q-A2", "Overblow by continuous interpolation of the lip setting between partials (f_l, h0 and/or p_m) produces silent gaps > 50 ms between registers for 10-19 of 35 notes and non-monotonic partial sequences, so it is rejected.",
     "research/spikes/overblow_spike.py", "overblow_spike_result.json", ["D-009"], ["brass-regime-selection"]),
    ("Q-A2", "Register jumps implemented as a 20 ms ramp of (f_l, h0, p_m) from the setting of partial n to that of partial m succeed for 131/133 tested jumps (n+1, n+2, n-1, n-2, including down to the pedal partial 1); all jumps in the normal range succeed; settling time median 45 ms, max 145 ms; failures: written 86 p9 -> p11 and 87 p10 -> p12.",
     "research/spikes/overblow_d.py", "overblow_d_result.json", ["D-009", "T-015"], ["brass-regime-selection"]),
    ("Q-A2", "Within a register, raising only the blowing pressure (p_m x (1 + 2 o) for o in [0, 0.45]) keeps 35/35 standard notes on their partial and increases the mouthpiece-pressure centroid monotonically for 35/35; adding f_l x (1 + 0.04 o) pushes 11/35 notes to the next partial early.",
     "research/spikes/brighten_spike.py", "session output 2026-10-07", ["D-009", "T-015"], ["brass-regime-selection"]),
    ("Q-A6", "Output chain p+ = p/2 -> simple-wave time warping over 0.85 m of equivalent cylinder -> TMM bell transfer j w H / Z_rad gives a mean harmonic MAD (H1..H8) vs TinySOL of 2.9 dB at ff (6 notes), but pp/mf are too bright (13.6 / 7.5 dB) when the steady pressure stays at 1.3 / 2.5 x threshold; without nonlinear propagation ff is too dark (centroid ratio 0.47-0.59).",
     "research/spikes/timbre_spike.py", "timbre_spike_result.json", ["D-012", "D-017", "T-029"], ["brass-nonlinear-propagation"]),
    ("Q-A8", "Relaxing the blowing pressure after the onset (start 1.3 x threshold, sustain 0.6-1.0 x threshold, using the subcritical hysteresis) lowers the mouthpiece amplitude and gives harmonic MAD vs TinySOL pp 4.3-6.6 dB and mf 2.3-4.6 dB on written 64, 74, 78; sustaining below ~0.8 x threshold loses the note on partial 6 (written 78); lip Ql 7-10 with the Doc set does not oscillate.",
     "research/spikes/dyn_spike.py", "session output 2026-10-07", ["D-008", "T-029"], ["trumpet-blowing-pressure"]),
    ("Q-A8", "Doc 2023 thresholds x dynamic factors (pp 1.3, mf 2.5, ff 5) give 3.2 / 6.2 / 12.4 kPa for written C5 (partial 4), matching Fletcher & Tarnopolsky 1999 (3.3 / 6.3 / 13 kPa, C-A20).",
     "research/spikes/regime_map.py", "PTH and DYN constants", ["D-008"], ["trumpet-blowing-pressure"]),
    ("Q-A1", "A 'tongue release' attack (lips closed at t = 0, 3 ms pressure rise) shortens the time to half the steady RMS (25 ms windows) from 0.17-0.57 s to 0.02-0.12 s, but by the reference onset metric it produces a 0-6 ms click-dominated onset (too fast vs TinySOL 22-164 ms), so the attack parameters (rise time, initial opening, accent) must be calibrated against the reference band.",
     "research/spikes/onset_spike.py", "session output 2026-10-07", ["D-007", "T-029"], ["brass-lip-model"]),
    ("Q-A1", "With the bell filter applied as a linear (zero-padded) convolution, the 'tongue release' attack gives onset medians of 40-160 ms by the reference metric (TinySOL band q10-q90: pp 26-164, mf 22-103, ff 22-99 ms); the best single setting (pressure rise 15 ms, lips closed, accent 1.5) puts 92/58/25 % of pp/mf/ff notes inside the band, and dynamic-dependent settings reach >= 50 % for every dynamic (pp 92 % at 15 ms/closed, mf 58 % at 15 ms/closed/accent 1.5, ff 50 % at 15 ms/half-open); >= 80 % per dynamic was not reached. Supersedes C-088 (circular-convolution artefact, lesson L-20261008T010500Z).",
     "research/spikes/onset_grid.py", "onset_grid_result.txt", ["D-007", "T-029"], ["brass-lip-model"]),
    ("Q-E6", "The planning sandbox (Claude Code cloud, Ubuntu 24.04) has no sudo, no system pip/ensurepip and no cmake; a venv created with --without-pip + get-pip.py, CMake 4.4.4 and Ninja 1.13.2 release binaries work; JUCE Linux -dev packages except libx11-dev and xvfb are missing.",
     "https://bootstrap.pypa.io/get-pip.py", "Phase 0.1 diagnostic", ["D-022"], ["claude-code-cloud-sandbox"]),
]


def tier_of(src):
    return int(src.get("tier", 3)) if isinstance(src, dict) else 3


def main():
    claims, n = [], 0
    for letter in ("E", "A", "F"):
        for item in json.load(open(ROOT / f"research/sources/agent-{letter}.json")):
            if item["id"] in ("F-TABLE", "F-ALT"):
                continue
            n += 1
            q = item.get("question", "")
            claims.append({
                "id": f"C-{n:03d}", "source_item": item["id"], "question": q, "claim": item["claim"], "kind": KIND[letter],
                "load_bearing": item["id"] not in NOT_LOAD_BEARING, "sources": item.get("sources", []),
                "confidence": item.get("confidence", "single-source"), "evidence_class": "n/a",
                "contradicted_by": [], "informs": INFORMS.get(q, []), "notes": item.get("notes", ""),
                "tags": TAGS[letter] + [f"subject:{SUBJECT.get(q, 'trumpet-vst')}"]})
    for q, claim, url, loc, informs, subj in SPIKES:
        n += 1
        claims.append({
            "id": f"C-{n:03d}", "source_item": "planner-spike", "question": q, "claim": claim, "kind": "numerical",
            "load_bearing": True,
            "sources": [{"url": url, "title": "planner spike", "version": "2026-10-07", "locator": loc, "accessed": "2026-10-07", "tier": 1}],
            "confidence": "verified", "evidence_class": "n/a", "contradicted_by": [], "informs": informs,
            "tags": ["domain:audio", "domain:musical-acoustics", "domain:brass"] + [f"subject:{s}" for s in subj]})
    # R3/R4 resolutions: arithmetic facts verified by computation; claims superseded by spike-based design decisions.
    for c in claims:
        if c["source_item"] in ("A-22", "F-1", "F-14"):
            c["confidence"] = "verified"
            c["sources"].append({"url": "research/spikes/expected_values.py", "title": "computation", "version": "2026-10-07",
                                 "locator": "values(): hs_dev_p*, open_partial_written_p*, ideal_sharp_*", "accessed": "2026-10-07", "tier": 1})
        if c["id"] == "C-088":
            c["load_bearing"] = False
            c["contradicted_by"] = ["C-090"]
            c["superseded_by"] = "C-090"
            c["notes"] = "Superseded: the 0-6 ms onsets were an artefact of circular FFT filtering (lesson L-20261008T010500Z)."
        if c["source_item"] in ("E-26", "E-29"):
            c["load_bearing"] = True
            c["confidence"] = "corroborated"
            c["sources"].append({"url": "https://github.com/qualitycoding/clarinet-vst/blob/impl/clarinet-v1/REPORT.md",
                                 "title": "clarinet-vst REPORT (sibling project CI on JUCE 9.0.3)", "version": "ef14e98",
                                 "locator": "SC-1 row: CI plugin job green on Linux (xvfb), macOS (auval PASS), Windows",
                                 "accessed": "2026-10-07", "tier": 2})
        if c["source_item"] == "A-17":
            c["load_bearing"] = False
            c["notes"] = (c.get("notes", "") + " | R3: superseded for design purposes by the TMM-tuned loops (planner spike, "
                          "make_table.py); no measured loop lengths are needed (D-004).").strip(" |")
        if c["source_item"] == "A-19":
            c["load_bearing"] = False
            c["notes"] = (c.get("notes", "") + " | R3: the radiation transfer is computed by the TMM (D-012), so this "
                          "single-source statement only corroborates its shape.").strip(" |")
    (ROOT / "research/claims.json").write_text(json.dumps(claims, indent=1, ensure_ascii=False) + "\n", encoding="utf-8")

    kdir = ROOT / "knowledge"
    kdir.mkdir(exist_ok=True)
    for old in kdir.glob("K-20261007T16*.md"):
        old.unlink()
    kids = {}
    for i, c in enumerate(claims):
        mm, ss = divmod(i, 60)
        kids[c["id"]] = (f"K-20261007T16{mm:02d}{ss:02d}Z-{re.sub(r'[^a-z0-9]+', '-', c['id'].lower())}", mm, ss)
    superseders = {c["superseded_by"]: c["id"] for c in claims if c.get("superseded_by")}
    for i, c in enumerate(claims):
        s0 = c["sources"][0] if c["sources"] else {}
        kid, mm, ss = kids[c["id"]]
        status = "superseded" if c.get("superseded_by") else "current"
        sup = [kids[superseders[c["id"]]][0]] if c["id"] in superseders else []
        src = {k: s0.get(k, "") for k in ("url", "doi", "title", "locator", "accessed", "tier")}
        stmt = c["claim"].replace("\n", " ").replace('"', "'")
        version = s0.get("version", "") or "version-independent"
        text = f"""---
id: {kid}
statement: "{stmt}"
status: {status}
supersedes: {json.dumps(sup)}
tags: [{', '.join(c['tags'] + ['phase:research'])}]
applies_to_version: "{version}"
source: {json.dumps(src, ensure_ascii=False)}
confidence: {c['confidence']}
derived_from: [{c['id']}]
discovered_by: planner
run: {RUN}
recorded_at: 2026-10-07T16:{mm:02d}:{ss:02d}Z
---
{c.get('notes', '') or 'See research/claims.json ' + c['id'] + '.'}
"""
        (kdir / f"{kid}.md").write_text(text, encoding="utf-8")
    print(len(claims), "claims;", len(claims), "knowledge items")


if __name__ == "__main__":
    main()
