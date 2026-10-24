#!/usr/bin/env python3
"""S-004 cross-check: the shipped C++ YIN (tpt_f0) against the partial each row of regime_map_result.json locked to.
Re-renders each row with research/spikes/lipsim.cpp exactly as sim() in regime_map.py does (fscale 1.0, mf).
Env: LIPSIM (path of the built lipsim binary; built here if missing), TMPDIR_SPIKE (scratch dir)."""
import json, os, pathlib, subprocess, sys
import numpy as np

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPIKES = ROOT / "research/spikes"
TMP = pathlib.Path(os.environ.get("TMPDIR_SPIKE", "/tmp/tpt_yin_crosscheck"))
LIPSIM = pathlib.Path(os.environ.get("LIPSIM", TMP / "lipsim"))
TPT_F0 = ROOT / "build/tools/render/tpt_f0"
FS = 48000

sys.path.insert(0, str(SPIKES))
import regime_map as rm   # noqa: E402  (pth, h0, ratio, poles_file helpers)

TMP.mkdir(parents=True, exist_ok=True)
if not LIPSIM.exists():
    subprocess.run(["g++", "-O2", "-std=c++20", "-o", str(LIPSIM), str(SPIKES / "lipsim.cpp")], check=True)
rm.S, rm.TMP = str(LIPSIM), str(TMP)

table = json.load(open(SPIKES / "table_draft.json"))
rows = json.load(open(SPIKES / "regime_map_result.json"))
out, agree, bad, skipped = [], 0, [], 0
for row in rows:
    if row["mf_partial"] == 0:
        skipped += 1
        continue
    st, n = row["state"], row["partial"]
    fres = [m["f_hz"] for m in table["states"][st]]
    raw = TMP / "x.raw"
    args = [str(LIPSIM), rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1]}", f"pm={2.5 * rm.pth(n)}", "Ql=20", "mu=9",
            "b=12e-3", f"H={rm.h0(n)}", "dur=2.0", "attack=0.003", "yinit=0", "fscale=1.0", "fmax=2000", "fmin=40",
            f"wav={raw}", "raw=1"]
    subprocess.run(args, capture_output=True, text=True, check=True)
    f0 = float(subprocess.run([str(TPT_F0), str(raw), str(FS)], capture_output=True, text=True, check=True).stdout)
    got = rm.partial_of(f0, fres) if f0 > 0 else 0
    ok = got == row["mf_partial"]
    agree += ok
    if not ok:
        bad.append((row["written"], row["valves"], row["kind"], row["mf_partial"], got, f0))
    out.append(f'| {row["written"]} | {row["valves"]} | {row["kind"]} | {row["mf_partial"]} | {got} | {f0:.2f} | {"yes" if ok else "NO"} |')

lines = ["# S-004 YIN cross-check", "",
         f"Rows: {len(rows)}; excluded (mf_partial 0): {skipped}; compared: {len(out)}; agreeing: {agree}; disagreeing: {len(bad)}.", "",
         "| written | valves | kind | spike partial | tpt_f0 partial | tpt_f0 Hz | agree |", "|---|---|---|---|---|---|---|"] + out
(ROOT / "logs/S-004-yin-crosscheck.md").write_text("\n".join(lines) + "\n")
print(lines[2]); [print("DISAGREE", b) for b in bad]
sys.exit(0 if len(bad) <= 2 else 1)
