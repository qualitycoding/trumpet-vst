"""Spike (L-20261007T150400Z): attack parameter grid. Pressure rise time t_r, initial lip opening y_init / h0,
accent k_acc (pressure starts at k_acc x sustain, relaxes over 150 ms). Reference onset metric
(ref_spread.onset_ms) on the radiated output (L_nl = 0.85 m). Target: per dynamic, synthetic onsets inside the
TinySOL [q10, q90] band (pp 26-164, mf 22-103, ff 22-99 ms; ref_spread_result.json) for >= 80 % of notes.
Every third standard note is used. The bell filter is applied as a zero-padded (linear) convolution.

Usage: python onset_grid.py table.json regime_map_result.json
"""
import json, os, subprocess, sys, itertools
import numpy as np
import regime_map as rm, ref_spread as ref, timbre_spike as ts, tmm_trumpet as t

BAND = {"pp": (25.8, 164.4), "mf": (21.8, 103.2), "ff": (21.7, 99.3)}
KSUS = {"pp": 0.8, "mf": 1.3, "ff": 5.0}


def render(table, note, fsc, pm_sus, k_acc, t_r, yfrac):
    st, n = note["state"], note["partial"]
    fres = [x["f_hz"] for x in table["states"][st]]
    path = os.path.join(rm.TMP, "og.raw")
    pm_on = max(k_acc * pm_sus, 1.3 * rm.pth(n))
    args = [rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fsc}", f"pm={pm_on}", f"pm_end={pm_sus}",
            f"ramp_start={t_r}", "ramp_dur=0.15", "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", "dur=0.6",
            f"attack={t_r}", f"yinit={yfrac * rm.h0(n)}", f"fscale={fsc}", f"wav={path}", "raw=1"]
    subprocess.run(args, capture_output=True, text=True)
    return np.fromfile(path, dtype=np.float32).astype(float)


if __name__ == "__main__":
    table = json.load(open(sys.argv[1]))
    reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    fit = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "bore_fit_result.json")))
    t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    notes = [n for n in table["notes"] if n["kind"] == "std" and reg[(n["written"], "std")]["fscale"]][::3]
    radi = {}
    for t_r, yfrac, k_acc in itertools.product((0.005, 0.015, 0.03), (0.0, 0.5, 1.0), (1.0, 1.5)):
        cov, meds = {}, {}
        for dyn in ("pp", "mf", "ff"):
            ons = []
            for note in notes:
                p = render(table, note, reg[(note["written"], "std")]["fscale"], KSUS[dyn] * rm.pth(note["partial"]), k_acc, t_r, yfrac)
                N = len(p)
                key = (note["valves"], 4 * N)
                if key not in radi:
                    radi[key] = ts.radiation(fit["geometry"], table["loops_m"], note["valves"], 4 * N)
                # zero-padded (4x) linear convolution: a circular FFT filter wraps the loud note end onto t = 0
                y = np.fft.irfft(np.fft.rfft(ts.nlp(p / 2, 0.85, t.RHO, t.C0), 4 * N) * radi[key], 4 * N)[:N]
                ons.append(ref.onset_ms(y, 48000))
            lo, hi = BAND[dyn]
            ons = np.array(ons)
            cov[dyn] = round(float(np.mean((ons >= lo) & (ons <= hi))), 2)
            meds[dyn] = float(np.median(ons))
        print(f"t_r {t_r * 1000:.0f} ms y0 {yfrac} acc {k_acc}: coverage {cov} medians {meds}", flush=True)
