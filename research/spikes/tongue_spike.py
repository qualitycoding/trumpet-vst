"""Spike (pre-mortem R-029): re-articulation by a tongue stop. A sustained note at mf; at t = 0.6 s the lips are held
closed and p_m = 0 for `stop` seconds, then the pressure re-attacks over 3 ms (D-007 tongue release). Measures, on the
radiated output (zero-padded bell transfer, L_nl 0.85 m), the depth of the 5 ms RMS dip after the stop (dB below the
steady level of 0.4-0.6 s) and the recovery time to within 3 dB of steady. Also a 100 ms gap (stop 0.1 s)."""
import json, os, subprocess, sys
import numpy as np
import regime_map as rm, timbre_spike as ts, tmm_trumpet as t

FS = 48000

def render(table, note, fsc, stop):
    st, n = note["state"], note["partial"]; fres = [x["f_hz"] for x in table["states"][st]]
    path = os.path.join(rm.TMP, "tongue.raw")
    args = [rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fsc}", f"pm={2.5 * rm.pth(n)}", "Ql=20", "mu=9",
            "b=12e-3", f"H={rm.h0(n)}", "dur=1.2", "attack=0.003", "yinit=0", f"fscale={fsc}", f"wav={path}", "raw=1",
            "stop_at=0.6", f"stop_dur={stop}"]
    subprocess.run(args, capture_output=True, text=True)
    return np.fromfile(path, dtype=np.float32).astype(float)

def env_db(y):
    n = int(0.005 * FS); m = len(y) // n
    e = np.sqrt(np.mean(y[: m * n].reshape(m, n) ** 2, axis=1)) + 1e-12
    return 20 * np.log10(e), n

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    fit = json.load(open("bore_fit_result.json")); t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    for stop in (0.015, 0.025, 0.04, 0.1):
        dips, recs = [], []
        for note in [x for x in table["notes"] if x["kind"] == "std" and x["written"] <= 84]:
            fsc = reg[(note["written"], "std")]["fscale"]
            p = render(table, note, fsc, stop)
            N = len(p); T = ts.radiation(fit["geometry"], table["loops_m"], note["valves"], 4 * N)
            y = np.fft.irfft(np.fft.rfft(ts.nlp(p / 2, 0.85, t.RHO, t.C0), 4 * N) * T, 4 * N)[:N]
            e, n = env_db(y)
            steady = np.median(e[int(0.4 * FS / n):int(0.6 * FS / n)])
            win = e[int(0.6 * FS / n):int((0.6 + stop + 0.06) * FS / n)]
            dips.append(steady - win.min())
            after = e[int((0.6 + stop) * FS / n):]
            k = next((i for i in range(len(after)) if after[i] >= steady - 3 and all(after[i:i + 10] >= steady - 3)), None)
            recs.append(k * n / FS * 1000 if k is not None else 999)
        dips, recs = np.array(dips), np.array(recs)
        print(f"stop {stop*1000:.0f} ms: dip dB min/median/max {dips.min():.1f}/{np.median(dips):.1f}/{dips.max():.1f}; "
              f"recovery ms median/max {np.median(recs):.0f}/{recs.max():.0f}; notes recovering <= 250 ms: {np.mean(recs <= 250):.2f}", flush=True)
