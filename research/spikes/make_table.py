"""Spike (reference implementation for tools/resonator/generate_table.py): resonator table for the B-flat trumpet.

1. Bore = the fitted geometry of bore_fit.py (bore_fit_result.json), air at 27 degC.
2. Valve loops (cylinders at the valve position) are tuned by bisection so that each valve alone lowers partials
   3..6 by exactly 1 (valve 2), 2 (valve 1), 3 (valve 3) equal-tempered semitones on average (D-BORE).
3. Every combination is computed by the TMM (loops add), giving the natural combination tendencies.
4. "Open-horn calibration": for mode index k, frequency ratio and peak-height ratio measured/TMM of the open horn
   (Freour 2022 Table 1, k = 1..11) are applied to mode k of every bore state; k > 11 uses the k = 11 ratios.
5. Slide-trigger variants (3rd slide for 13/123, 1st slide for flagged notes) extend the corresponding loop so the
   target partial's resonance has the same deviation from equal temperament as that partial of the open horn.
6. Each mode -> complex pole s = -w/(2Q) + j w sqrt(1 - 1/(4Q^2)) and dimensional residue R (p = 2 Re sum q,
   q' = s q + R u), as consumed by lipsim.cpp ("poles" format) and the C++ voice.

Usage: python make_table.py <fingerings.txt> <out.json>
"""
import json, sys
import numpy as np
import tmm_trumpet as t
import bore_fit as b

MEAS_POLES = [l.split() for l in open(__file__.replace("make_table.py", "freour2022_open_modes.txt")).read().split("\n")[1:] if l.strip()]
F = np.linspace(40, 2000, 7841)
NMODES = 14
VALVE_SEMITONES = {"2": 1, "1": 2, "3": 3}


def state_peaks(geo, loops, combo, trig_extra=0.0):
    extra = sum(loops[v] for v in combo if v != "0") + trig_extra
    segs = t.segments(geo["profile"], geo["valve_x_m"], extra, geo["valve_r_m"])
    Z, _ = t.zin_and_h(F, segs)
    return t.peaks(F, Z / t.zc(b.R_CUP))[:NMODES]


def tune_loops(geo):
    open_pk = state_peaks(geo, {}, "0")
    loops = {}
    for v, st in VALVE_SEMITONES.items():
        lo, hi = 0.0, 0.6
        for _ in range(50):
            mid = 0.5 * (lo + hi)
            pk = state_peaks(geo, {v: mid}, v)
            low = np.mean([1200 * np.log2(open_pk[k][0] / pk[k][0]) for k in range(2, 6)])
            lo, hi = (mid, hi) if low < 100 * st else (lo, mid)
        loops[v] = 0.5 * (lo + hi)
    return loops, open_pk


def calib(open_pk):
    zc = t.zc(b.R_CUP)
    fr, hr = [], []
    for k in range(NMODES):
        if k < len(MEAS_POLES):
            s = complex(float(MEAS_POLES[k][0]), float(MEAS_POLES[k][1]))
            C = complex(float(MEAS_POLES[k][2]), float(MEAS_POLES[k][3]))
            fr.append((s.imag / (2 * np.pi)) / open_pk[k][0])
            hr.append(abs(C / -s.real) / zc / open_pk[k][1])
        else:
            fr.append(fr[-1]); hr.append(hr[-1])
    return fr, hr


def to_poles(pk, fr, hr):
    zc = t.zc(b.R_CUP)
    out = []
    for k, (f, A, Q) in enumerate(pk):
        f, A = f * fr[k], A * hr[k]
        wn = 2 * np.pi * f
        z = 1 / (2 * Q)
        sig = complex(-z * wn, wn * np.sqrt(1 - z * z))
        R = zc * A * (wn / Q) * sig / (sig - sig.conjugate())
        out.append({"k": k + 1, "f_hz": round(f, 4), "Q": round(Q, 4), "peak_over_zc": round(A, 4),
                    "s": [round(sig.real, 6), round(sig.imag, 6)], "R": [round(R.real, 2), round(R.imag, 2)]})
    return out


def et_hz(concert_midi):
    return 440.0 * 2 ** ((concert_midi - 69) / 12)


def read_fingerings(path):
    rows = []
    for line in open(path):
        line = line.split("#")[0].strip()
        if not line:
            continue
        # written_midi valves partial trigger(-,1,3) kind(std,alt)
        w, valves, p, trig, kind = line.split()
        rows.append({"written": int(w), "valves": valves, "partial": int(p.lstrip("p")), "trigger": trig, "kind": kind})
    return rows


if __name__ == "__main__":
    fit = json.load(open(__file__.replace("make_table.py", "bore_fit_result.json")))
    geo = fit["geometry"]
    loops, open_pk = tune_loops(geo)
    fr, hr = calib(open_pk)
    combos = ["0", "1", "2", "3", "12", "13", "23", "123"]
    states = {}
    for c in combos:
        states[c] = to_poles(state_peaks(geo, loops, c), fr, hr)
    # open-horn deviation of partial n from ET (reference for trigger tuning)
    open_dev = {k + 1: 1200 * np.log2(states["0"][k]["f_hz"] / et_hz(46 + 12 * np.log2(k + 1))) for k in range(NMODES)}
    rows = read_fingerings(sys.argv[1])
    notes = []
    for r in rows:
        concert = r["written"] - 2
        n = r["partial"]
        key = r["valves"]
        lower = sum(VALVE_SEMITONES[v] for v in key if v != "0")
        # equal-tempered pitch of partial n of this combination: open partial pitch - lowering
        target = et_hz(concert)
        st = states[key]
        dev_nat = 1200 * np.log2(st[n - 1]["f_hz"] / target)
        state_id = key
        trig_len = 0.0
        if r["trigger"] != "-":
            v = r["trigger"]
            want = open_dev[n]  # same deviation as the open horn's partial n
            lo, hi = 0.0, 0.08
            for _ in range(40):
                mid = 0.5 * (lo + hi)
                pk = state_peaks(geo, loops, key, mid)
                d = 1200 * np.log2(pk[n - 1][0] * fr[n - 1] / target)
                lo, hi = (mid, hi) if d > want else (lo, mid)
            trig_len = 0.5 * (lo + hi)
            state_id = f"{key}+t{v}@{r['written']}"
            states[state_id] = to_poles(state_peaks(geo, loops, key, trig_len), fr, hr)
        dev = 1200 * np.log2(states[state_id][n - 1]["f_hz"] / target)
        notes.append({**r, "concert": concert, "state": state_id, "trigger_len_m": round(trig_len, 5),
                      "res_dev_cents_natural": round(dev_nat, 2), "res_dev_cents": round(dev, 2)})
    out = {"air": fit["air"], "loops_m": loops, "calib_freq_ratio": fr, "calib_height_ratio": hr,
           "states": states, "notes": notes}
    json.dump(out, open(sys.argv[2], "w"), indent=1)
    print("loops (m):", {k: round(v, 4) for k, v in loops.items()})
    for c in combos:
        print(c, "partials 2..8 dev from ET (c):",
              [round(1200 * np.log2(states[c][k]["f_hz"] / et_hz(46 + 12 * np.log2(k + 1) - sum(VALVE_SEMITONES[v] for v in c if v != '0'))), 1) for k in range(1, 8)])
    for nrow in notes:
        print(nrow["written"], nrow["valves"], "p%d" % nrow["partial"], nrow["trigger"], nrow["kind"],
              "nat %+.1f c -> %+.1f c" % (nrow["res_dev_cents_natural"], nrow["res_dev_cents"]))
