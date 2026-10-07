"""Spike (L-20261007T150700Z): for every fingering in the table, simulate the lip model at the default (overblow = 0)
lip setting for its partial and record which partial it locks to, its pitch offset and onset; then calibrate a
per-note frequency scale so it plays at equal temperament; then sweep Overblow/Underblow.

Lip model: Doc, Vergez & Hannebicq 2023 (JASA 153(1)) Table II set: W = 12 mm, Ql = 20, mu = 9 kg/m^2;
per-partial h0 and f_l/f_res (Table II registers 2..6; other partials extrapolated, D-LIP).
Pressure: threshold per partial (Doc 2023 Table I medians, extrapolated) x dynamic factor (pp 1.3, mf 2.5, ff 5).
Attack: "tongue release" (yinit = 0, 3 ms pressure rise).

Usage: python regime_map.py table.json out.json [--quick]
"""
import json, os, subprocess, sys
import numpy as np

S = os.environ.get("LIPSIM", "/tmp/claude-1000/-home-claude-projects-trumpet-vst/f855d006-2b49-5a65-8b7d-22066c71447d/scratchpad/lipsim")
TMP = os.environ.get("TMPDIR_SPIKE", "/tmp/claude-1000/-home-claude-projects-trumpet-vst/f855d006-2b49-5a65-8b7d-22066c71447d/scratchpad/states")
os.makedirs(TMP, exist_ok=True)

# Doc 2023 Table II (registers 2..6) on their trumpet: f_l and h0; f_res of their instrument is not tabulated, so the
# ratio is taken against the measured Freour 2022 resonances of the same nominal registers (C-LIP-RATIO, inferred).
H0_MM = {1: 0.27, 2: 0.242, 3: 0.218, 4: 0.190, 5: 0.172, 6: 0.168}
RATIO = {1: 1.01, 2: 235 / 232.7, 3: 340 / 348.07, 4: 467 / 462.6, 5: 586 / 582.14, 6: 703 / 690.58}
PTH = {1: 800, 2: 1047, 3: 1788, 4: 2477, 5: 3158, 6: 4109}   # Doc 2023 Table I medians (p1 extrapolated)


def h0(n):
    return (H0_MM[n] if n in H0_MM else max(0.12, 0.168 - 0.006 * (n - 6))) * 1e-3


def ratio(n):
    return RATIO[n] if n in RATIO else RATIO[6]


def pth(n):
    return PTH[n] if n in PTH else 4109 + 700 * (n - 6)


DYN = {"pp": 1.3, "mf": 2.5, "ff": 5.0}


def poles_file(table, state):
    path = os.path.join(TMP, state.replace("+", "_").replace("@", "_") + ".txt")
    if not os.path.exists(path):
        with open(path, "w") as fh:
            fh.write("poles\n")
            for m in table["states"][state]:
                fh.write("%r %r %r %r\n" % (m["s"][0], m["s"][1], m["R"][0], m["R"][1]))
    return path


def sim(table, state, n, pm, fscale=1.0, lam_end=None, dur=0.8, track=False):
    st = table["states"][state]
    fres = [m["f_hz"] for m in st]
    fl0 = ratio(n) * fres[n - 1] * fscale
    args = [S, poles_file(table, state), f"fl={fl0}", f"pm={pm}", "Ql=20", "mu=9", "b=12e-3", f"H={h0(n)}",
            f"dur={dur}", "attack=0.003", "yinit=0", f"fscale={fscale}", "fmax=2000", "fmin=40"]
    if lam_end is not None:
        m = lam_end
        lo, hi = int(np.floor(m)), int(np.ceil(m))
        w = m - lo
        fl1 = np.exp((1 - w) * np.log(ratio(lo) * fres[lo - 1]) + w * np.log(ratio(hi) * fres[hi - 1])) * fscale
        h1 = (1 - w) * h0(lo) + w * h0(hi)
        args += [f"fl_end={fl1}", f"H_end={h1}"]
    if track:
        args.append("track=1")
    out = subprocess.run(args, capture_output=True, text=True).stdout.splitlines()
    rows = [json.loads(l) for l in out]
    if track:
        return rows, fres
    return rows[-1], fres


def partial_of(f0, fres, fscale=1.0):
    if f0 <= 0:
        return 0
    return int(np.argmin([abs(np.log(f0 / (f * fscale))) for f in fres])) + 1


def onset_s(table, state, n, pm, fscale):
    rows, fres = sim(table, state, n, pm, fscale, track=True, dur=0.6)
    steady = np.median([r["rms"] for r in rows[-6:]])
    for r in rows:
        if r["rms"] >= 0.5 * steady:
            return r["t"]
    return None


def calibrate(table, state, n, target, pm):
    fs = 1.0
    for _ in range(5):
        r, fres = sim(table, state, n, pm, fs)
        if r.get("rms", 0) < 20 or r.get("f0", 0) <= 0:
            return None
        c = 1200 * np.log2(r["f0"] / target)
        if abs(c) < 0.5:
            break
        fs *= 2 ** (-c / 1200)
    return fs, c


if __name__ == "__main__":
    table = json.load(open(sys.argv[1]))
    quick = "--quick" in sys.argv
    res = []
    for note in table["notes"]:
        st, n = note["state"], note["partial"]
        target = 440 * 2 ** ((note["concert"] - 69) / 12)
        row = {"written": note["written"], "valves": note["valves"], "partial": n, "kind": note["kind"], "state": st}
        for dyn, k in DYN.items():
            r, fres = sim(table, st, n, k * pth(n))
            row[f"{dyn}_partial"] = partial_of(r.get("f0", 0), fres) if r.get("rms", 0) > 20 else 0
            row[f"{dyn}_cents_vs_et"] = round(1200 * np.log2(r["f0"] / target), 1) if r.get("f0", 0) > 0 else None
        cal = calibrate(table, st, n, target, DYN["mf"] * pth(n))
        row["fscale"] = round(cal[0], 6) if cal else None
        if cal and not quick:
            fs = cal[0]
            for dyn, k in DYN.items():
                r, fres = sim(table, st, n, k * pth(n), fs)
                row[f"{dyn}_cal_cents"] = round(1200 * np.log2(r["f0"] / target), 1) if r.get("f0", 0) > 0 else None
            row["onset_mf_s"] = onset_s(table, st, n, DYN["mf"] * pth(n), fs)
            # overblow: lip setting moves from partial n to n+2 over 1.2 s (continuous); underblow to max(1, n-2)
            for name, lam in (("over", min(n + 2, 14)), ("under", max(1, n - 2))):
                rows, fres = sim(table, st, n, DYN["mf"] * pth(n), fs, lam_end=lam, dur=1.2, track=True)
                seq = [partial_of(x["f0"], fres, fs) if x["rms"] > 20 and x["clarity"] > 0.8 else 0 for x in rows]
                row[f"{name}_partials"] = seq
        res.append(row)
        print(json.dumps({k: v for k, v in row.items() if not k.endswith("_partials")}), flush=True)
    json.dump(res, open(sys.argv[2], "w"), indent=1)
