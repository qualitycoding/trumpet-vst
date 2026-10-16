"""Spike (T-035 b feasibility): D-007 release (p_m falls to 0 over 40 ms, here a linear ramp, slightly slower than the
raised cosine at the start) after a sustained mf note; minimum 5 ms RMS of the radiated output within the 100 ms after
note-off, in dB below the steady level."""
import json, os, subprocess, sys
import numpy as np
import regime_map as rm, timbre_spike as ts, tmm_trumpet as t
import tongue_spike as tg

FS = 48000
if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    fit = json.load(open("bore_fit_result.json")); t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    dips = []
    for note in [x for x in table["notes"] if x["kind"] == "std" and x["written"] <= 84]:
        st, n = note["state"], note["partial"]; fres = [x["f_hz"] for x in table["states"][st]]; fsc = reg[(note["written"], "std")]["fscale"]
        path = os.path.join(rm.TMP, "rel.raw")
        subprocess.run([rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fsc}", f"pm={2.5 * rm.pth(n)}", "pm_end=0",
                        "ramp_start=0.6", "ramp_dur=0.04", "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", "dur=0.8", "attack=0.003",
                        "yinit=0", f"fscale={fsc}", f"wav={path}", "raw=1"], capture_output=True, text=True)
        p = np.fromfile(path, dtype=np.float32).astype(float); N = len(p)
        T = ts.radiation(fit["geometry"], table["loops_m"], note["valves"], 4 * N)
        y = np.fft.irfft(np.fft.rfft(ts.nlp(p / 2, 0.85, t.RHO, t.C0), 4 * N) * T, 4 * N)[:N]
        e, w = tg.env_db(y)
        steady = np.median(e[int(0.4 * FS / w):int(0.6 * FS / w)])
        dips.append(steady - e[int(0.6 * FS / w):int(0.7 * FS / w)].min())
    d = np.array(dips)
    print(f"release 40 ms, 100 ms window: dip dB min/median/max {d.min():.1f}/{np.median(d):.1f}/{d.max():.1f}; >= 20 dB: {np.mean(d >= 20):.2f}; >= 12 dB: {np.mean(d >= 12):.2f}")
