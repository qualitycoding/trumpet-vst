"""Spike: soft dynamics. Start at k_on x threshold, then relax pressure to k_sus x threshold (0.1 s .. 0.3 s);
also lip quality factor variants. Reports mouthpiece peak pressure and harmonic MAD vs TinySOL pp/mf with and without
nonlinear propagation."""
import json, os, subprocess, sys, numpy as np
import regime_map as rm, ref_spread as ref, timbre_spike as ts, tmm_trumpet as t

def render(table, note, fsc, pm_on, pm_sus, Ql=20, mu=9):
    st, n = note["state"], note["partial"]; fres = [x["f_hz"] for x in table["states"][st]]
    path = os.path.join(rm.TMP, "dyn.raw")
    args = [rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fsc}", f"pm={pm_on}", f"pm_end={pm_sus}",
            "ramp_start=0.1", "ramp_dur=0.2", f"Ql={Ql}", f"mu={mu}", "b=12e-3", f"H={rm.h0(n)}", "dur=1.2", "attack=0.003",
            "yinit=0", f"fscale={fsc}", f"wav={path}", "raw=1"]
    r = json.loads(subprocess.run(args, capture_output=True, text=True).stdout.splitlines()[-1])
    return np.fromfile(path, dtype=np.float32).astype(float), r

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    refrows = {(r["midi"], r["dyn"]): r for r in ref.analyse(sys.argv[3])}
    fit = json.load(open("bore_fit_result.json")); t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    for w in (64, 74, 78):
        note = next(x for x in table["notes"] if x["written"] == w and x["kind"] == "std"); fsc = reg[(w, "std")]["fscale"]
        pth = rm.pth(note["partial"]); f0 = 440 * 2 ** ((note["concert"] - 69) / 12)
        T = None
        for (kon, ksus, Ql) in ((1.3, 1.3, 20), (1.3, 1.0, 20), (1.3, 0.8, 20), (1.3, 0.6, 20), (1.5, 1.0, 10), (1.5, 0.8, 10), (2.0, 1.2, 7)):
            p, r = render(table, note, fsc, kon * pth, ksus * pth, Ql)
            if T is None: T = ts.radiation(fit["geometry"], table["loops_m"], note["valves"], len(p))
            res = []
            for L in (0.0, 0.85):
                y = np.fft.irfft(np.fft.rfft(ts.nlp(p / 2, L, t.RHO, t.C0)) * T, len(p))[int(0.6 * 48000):]
                if np.max(np.abs(y)) < 1e-9: res.append("silent"); continue
                hdb, cen = ref.harmonics(y, 48000, f0)
                for dyn in ("pp", "mf"):
                    rr = refrows[(note["concert"], dyn)]
                    res.append(f"L{L}/{dyn}:{np.mean(np.abs(np.array(hdb[:8]) - np.array(rr['harm_db'][:8]))):.1f}dB,cr{cen / rr['centroid_hz']:.2f}")
            print(w, f"on{kon} sus{ksus} Ql{Ql}", "pk %.2f kPa" % (np.abs(p[int(0.6*48000):]).max() / 1000), "rms", round(r["rms"]), " ".join(res), flush=True)
