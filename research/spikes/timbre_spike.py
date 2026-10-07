"""Spike (L-20261007T150500Z): does lip model + modal bore + output-side nonlinear propagation + bell radiation reach
realistic spectra? Renders notes at pp/mf/ff and compares with TinySOL using the metrics of ref_spread.py.

Output chain (D-OUT): p_plus = p / 2 (outgoing wave at the mouthpiece, approximation) -> simple-wave nonlinear
propagation over L_nl of equivalent cylinder (time warping, t_arr = t - beta L_nl p+/(rho c^3), shocks resolved by
monotone arrival times) -> bell/radiation transfer T(f) = j w H(f) / Z_rad(f) from the TMM (H = p_bell/p_in) ->
level normalisation. Variants: L_nl in {0, 0.85} m.

Usage: python timbre_spike.py table.json regime_map_result.json <TinySOL root> out.json
"""
import json, os, subprocess, sys
import numpy as np
import soundfile as sf
import regime_map as rm
import ref_spread as ref
import tmm_trumpet as t
import bore_fit as b

FS = 48000
BETA = 1.2


def nlp(pp, L, rho, c):
    if L <= 0:
        return pp.copy()
    tk = np.arange(len(pp)) / FS - BETA * L * pp / (rho * c ** 3)
    tk = np.maximum.accumulate(tk)            # shock: arrival times cannot reverse
    return np.interp(np.arange(len(pp)) / FS, tk, pp)


def radiation(geo, loops, combo, n):
    f = np.fft.rfftfreq(n, 1 / FS)
    f[0] = 1.0
    extra = sum(loops[v] for v in combo if v != "0")
    segs = t.segments(geo["profile"], geo["valve_x_m"], extra, geo["valve_r_m"])
    _, H = t.zin_and_h(f, segs)
    r_end = geo["profile"][-1][1]
    T = 1j * 2 * np.pi * f * H / t.zrad(f, r_end)
    T[0] = 0
    return T


def render(table, note, fs_scale, pm):
    st, n = note["state"], note["partial"]
    fres = [x["f_hz"] for x in table["states"][st]]
    path = os.path.join(rm.TMP, "timbre.raw")
    args = [rm.S, rm.poles_file(table, st), f"fl={rm.ratio(n) * fres[n - 1] * fs_scale}", f"pm={pm}", "Ql=20", "mu=9",
            "b=12e-3", f"H={rm.h0(n)}", "dur=1.2", "attack=0.003", "yinit=0", f"fscale={fs_scale}", f"wav={path}", "raw=1"]
    subprocess.run(args, capture_output=True, text=True)
    return np.fromfile(path, dtype=np.float32).astype(float)


if __name__ == "__main__":
    table = json.load(open(sys.argv[1]))
    reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    refrows = {(r["midi"], r["dyn"]): r for r in ref.analyse(sys.argv[3])}
    fit = json.load(open(os.path.join(os.path.dirname(__file__), "bore_fit_result.json")))
    t.C0, t.RHO = fit["air"]["c"], fit["air"]["rho"]
    geo, loops = fit["geometry"], table["loops_m"]
    picks = [60, 64, 69, 74, 78, 81]
    out = []
    for L in (0.0, 0.85):
        for w in picks:
            note = next(x for x in table["notes"] if x["written"] == w and x["kind"] == "std")
            fsc = reg[(w, "std")]["fscale"]
            for dyn, k in rm.DYN.items():
                if (note["concert"], dyn) not in refrows:
                    continue
                p = render(table, note, fsc, k * rm.pth(note["partial"]))
                y = nlp(p / 2, L, t.RHO, t.C0)
                T = radiation(geo, loops, note["valves"], len(y))
                yo = np.fft.irfft(np.fft.rfft(y) * T, len(y))
                seg = yo[int(0.5 * FS):]
                f0 = 440 * 2 ** ((note["concert"] - 69) / 12)
                hdb, cen = ref.harmonics(seg, FS, f0)
                rr = refrows[(note["concert"], dyn)]
                mad = float(np.mean(np.abs(np.array(hdb[:8]) - np.array(rr["harm_db"][:8]))))
                row = {"L_nl": L, "written": w, "dyn": dyn, "peak_p_kpa": round(float(np.abs(p[int(0.5*FS):]).max()) / 1000, 2),
                       "harm_mad_db": round(mad, 2), "centroid_ratio": round(cen / rr["centroid_hz"], 3),
                       "ncent_syn": round(cen / f0, 2), "ncent_ref": rr["ncentroid"],
                       "h_syn": [round(v, 1) for v in hdb[:8]], "h_ref": rr["harm_db"][:8]}
                out.append(row)
                print(json.dumps(row), flush=True)
    json.dump(out, open(sys.argv[4], "w"), indent=1)
    for L in (0.0, 0.85):
        for dyn in ("pp", "mf", "ff"):
            rs = [r for r in out if r["L_nl"] == L and r["dyn"] == dyn]
            print(f"L_nl={L} {dyn}: mean MAD {np.mean([r['harm_mad_db'] for r in rs]):.1f} dB, centroid ratios {[r['centroid_ratio'] for r in rs]}")
