"""Spike: attack strategy. Tongue release (lips closed, 3 ms pressure rise) with an accent: p_m starts at k_acc x the
sustain pressure and relaxes to the sustain pressure over t_acc. Onset measured with the reference metric
(ref_spread.onset_ms) on the radiated output (L_nl = 0.85 m)."""
import json, os, subprocess, sys, numpy as np
import regime_map as rm, ref_spread as ref, timbre_spike as ts, tmm_trumpet as t

def render(table, note, fsc, pm_sus, k_acc, t_acc):
    st, n = note["state"], note["partial"]; fres = [x["f_hz"] for x in table["states"][st]]
    path = os.path.join(rm.TMP, "on.raw")
    args = [rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fsc}", f"pm={k_acc * pm_sus}", f"pm_end={pm_sus}",
            "ramp_start=0.003", f"ramp_dur={t_acc}", "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", "dur=0.7", "attack=0.003",
            "yinit=0", f"fscale={fsc}", f"wav={path}", "raw=1"]
    subprocess.run(args, capture_output=True, text=True)
    return np.fromfile(path, dtype=np.float32).astype(float)

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    fit = json.load(open("bore_fit_result.json")); t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    notes = [n for n in table["notes"] if n["kind"] == "std" and reg[(n["written"], "std")]["fscale"]]
    for k_acc, t_acc in ((1.0, 0.001), (2.0, 0.03), (3.0, 0.03), (3.0, 0.06)):
        for dyn, ksus in (("pp", 0.8), ("mf", 1.3), ("ff", 5.0)):
            ons = []
            for note in notes:
                fsc = reg[(note["written"], "std")]["fscale"]
                p = render(table, note, fsc, ksus * rm.pth(note["partial"]), max(k_acc, 1.3 / ksus if ksus < 1.3 else 1.0) if k_acc > 1 else max(1.0, 1.3 / ksus), t_acc)
                T = ts.radiation(fit["geometry"], table["loops_m"], note["valves"], 4 * len(p))
                y = np.fft.irfft(np.fft.rfft(ts.nlp(p / 2, 0.85, t.RHO, t.C0), 4 * len(p)) * T, 4 * len(p))[:len(p)]
                ons.append(ref.onset_ms(y, 48000))
            ons = np.array(ons)
            print(f"acc x{k_acc} {t_acc*1000:.0f} ms {dyn}: onset q10/med/q90 = {np.percentile(ons,10):.0f}/{np.median(ons):.0f}/{np.percentile(ons,90):.0f} ms, max {ons.max():.0f}", flush=True)
