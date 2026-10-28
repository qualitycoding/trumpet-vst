# SPDX-License-Identifier: Apache-2.0
"""Summarise results.json of compare_tinysol against the D-017 thresholds.

Usage: python tools/realism/summarize.py results.json --out SUMMARY.md
Per dynamic: harmonic MAD, centroid ratio coverage, onset median and the dynamics ordering; the 10 worst notes by
harm_mad_db, split by register (p2-p3, p4-p6, p8+) using the partial of the standard fingering of written = concert + 2."""
import argparse
import json
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path(__file__).resolve().parents[2]
THRESH = {"mad_mean_db": 6.0, "mad_note_db": 10.0, "mad_note_fraction": 0.90, "centroid_band": (0.75, 1.333),
          "centroid_fraction": 0.85, "onset_median_ms": 150.0, "order_fraction": 0.90}


def standard_partials(path=ROOT / "tests" / "fixtures" / "trumpet_fingerings.txt"):
    out = {}
    for line in pathlib.Path(path).read_text().splitlines():
        f = line.split("#")[0].split()
        if f and f[4] == "std":
            out[int(f[0])] = int(f[2][1:])
    return out


def register(partial):
    return "p2-p3" if partial <= 3 else ("p4-p6" if partial <= 6 else "p8+")


def summarise(rows):
    lines = ["# Realism summary (T-029 metrics, D-017)", "", f"{len(rows)} rows.", "",
             "| dynamic | notes | MAD mean (dB) | MAD <= 10 dB | centroid in band | onset median (ms) | verdict |", "|---|---|---|---|---|---|---|"]
    lo, hi = THRESH["centroid_band"]
    for dyn in ("pp", "mf", "ff"):
        rs = [r for r in rows if r["dyn"] == dyn]
        if not rs:
            continue
        mad = np.array([r["harm_mad_db"] for r in rs])
        cr = np.array([r["centroid_ratio"] for r in rs])
        on = np.array([r["onset_syn_ms"] for r in rs])
        ok = (mad.mean() <= THRESH["mad_mean_db"], np.mean(mad <= THRESH["mad_note_db"]) >= THRESH["mad_note_fraction"],
              np.mean((cr >= lo) & (cr <= hi)) >= THRESH["centroid_fraction"], np.median(on) <= THRESH["onset_median_ms"])
        lines.append(f"| {dyn} | {len(rs)} | {mad.mean():.2f} | {100 * np.mean(mad <= 10):.0f} % | {100 * np.mean((cr >= lo) & (cr <= hi)):.0f} % | "
                     f"{np.median(on):.0f} | {'pass' if all(ok) else 'FAIL'} |")
    by = {(r["concert"], r["dyn"]): r for r in rows}
    pitches = sorted({c for c, _ in by if all((c, d) in by for d in ("pp", "mf", "ff"))})
    ordered = [c for c in pitches if by[(c, "pp")]["ncentroid_syn"] < by[(c, "mf")]["ncentroid_syn"] < by[(c, "ff")]["ncentroid_syn"]]
    lines += ["", f"Centroid ordering pp < mf < ff: {len(ordered)} of {len(pitches)} pitches "
                  f"({'pass' if len(ordered) >= THRESH['order_fraction'] * len(pitches) else 'FAIL'}).", ""]
    parts = standard_partials()
    for reg in ("p2-p3", "p4-p6", "p8+"):
        sel = [r for r in rows if register(parts.get(r["concert"] + 2, 0)) == reg]
        worst = sorted(sel, key=lambda r: -r["harm_mad_db"])[:10]
        lines += [f"## Worst notes by harmonic MAD, register {reg}", "", "| concert | dyn | MAD (dB) | centroid ratio | onset (ms) |", "|---|---|---|---|---|"]
        lines += [f"| {r['concert']} | {r['dyn']} | {r['harm_mad_db']:.1f} | {r['centroid_ratio']:.2f} | {r['onset_syn_ms']:.0f} |" for r in worst]
        lines.append("")
    return "\n".join(lines) + "\n"


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("results")
    ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    rows = json.loads(pathlib.Path(a.results).read_text())
    pathlib.Path(a.out).write_text(summarise(rows))
    print(f"wrote {a.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
