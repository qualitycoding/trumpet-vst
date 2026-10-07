"""Spike: compare Overblow mappings on settled notes (0.3 s at the default setting, then a 1.5 s ramp to o = +1 or -1).
Mapping variants (lambda = n + 2 o is the 'aimed partial'):
  A  f_l, h0 and p_m all interpolated to the setting of partial lambda
  B  f_l only (log-interpolated to f_l of partial lambda); h0, p_m fixed
  C  f_l and p_m interpolated; h0 fixed
Scores: monotone (partial never decreases on overblow / increases on underblow), longest silent run (ms),
highest (lowest) partial reached."""
import itertools, json, sys
import numpy as np
import regime_map as rm

def run(table, note, o_end, var):
    st, n = note["state"], note["partial"]
    fres = [m["f_hz"] for m in table["states"][st]]
    fs = note["fscale"]
    lam = max(1.0, min(14.0, n + 2 * o_end))
    lo, hi = int(np.floor(lam)), int(np.ceil(lam)); w = lam - lo
    flx = lambda k: rm.ratio(k) * fres[k - 1]
    fl0 = flx(n) * fs
    fl1 = np.exp((1 - w) * np.log(flx(lo)) + w * np.log(flx(hi))) * fs
    pm0 = 2.5 * rm.pth(n); pm1 = 2.5 * ((1 - w) * rm.pth(lo) + w * rm.pth(hi))
    h00 = rm.h0(n); h01 = (1 - w) * rm.h0(lo) + w * rm.h0(hi)
    args = [rm.S, rm.poles_file(table, st), f"fl={fl0}", f"fl_end={fl1}", f"pm={pm0}", "Ql=20", "mu=9", "b=12e-3",
            f"H={h00}", "dur=1.8", "ramp_start=0.3", "attack=0.003", "yinit=0", f"fscale={fs}", "track=1", "fmax=2200", "fmin=40"]
    if var == "A": args += [f"pm_end={pm1}", f"H_end={h01}"]
    if var == "C": args += [f"pm_end={pm1}"]
    rows = [json.loads(l) for l in rm.subprocess.run(args, capture_output=True, text=True).stdout.splitlines()]
    seq = [rm.partial_of(x["f0"], fres, fs) if x["rms"] > 20 and x["clarity"] > 0.8 else 0 for x in rows if x["t"] > 0.3]
    return seq

def score(seq, up):
    nz = [s for s in seq if s]
    mono = all((b >= a) if up else (b <= a) for a, b in zip(nz, nz[1:]))
    sil = max([len(list(g)) for k, g in itertools.groupby(seq) if k == 0] or [0]) * 25
    ext = (max(nz) if up else min(nz)) if nz else 0
    return mono, sil, ext

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    notes = [dict(n, fscale=reg[(n["written"], n["kind"])]["fscale"]) for n in table["notes"] if n["kind"] == "std" and reg[(n["written"], n["kind"])]["fscale"]]
    res = {}
    for var in "ABC":
        tot = {"over_mono": 0, "under_mono": 0, "over_gap_ok": 0, "under_gap_ok": 0, "over_reach2": 0, "over_reach1": 0, "under_reach1": 0, "n": 0}
        for note in notes:
            n = note["partial"]
            so = run(table, note, 1.0, var); su = run(table, note, -1.0, var)
            mo, go, eo = score(so, True); mu, gu, eu = score(su, False)
            tot["n"] += 1; tot["over_mono"] += mo; tot["under_mono"] += mu
            tot["over_gap_ok"] += go <= 50; tot["under_gap_ok"] += gu <= 50
            tot["over_reach1"] += eo >= n + 1; tot["over_reach2"] += eo >= n + 2; tot["under_reach1"] += (eu <= n - 1 and eu > 0)
            res.setdefault(var, []).append({"written": note["written"], "n": n, "over": so, "under": su})
        print(var, tot, flush=True)
    json.dump(res, open(sys.argv[3], "w"))
