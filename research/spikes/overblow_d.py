"""Spike: Overblow mapping D. Register jump = 20 ms ramp of (f_l, h0, p_m) from the setting of partial n to that of
partial m (m = n+1, n+2, n-1, n-2), starting from a settled note (0.4 s). Measures: sounding partial 100 ms after the
jump, longest silent run during the jump (ms), transition time until the new partial is stable."""
import itertools, json, sys
import numpy as np
import regime_map as rm

def jump(table, note, m, dyn=2.5):
    st, n = note["state"], note["partial"]
    fres = [x["f_hz"] for x in table["states"][st]]
    fs = note["fscale"]
    flx = lambda k: rm.ratio(k) * fres[k - 1] * fs
    args = [rm.S, rm.poles_file(table, st), f"fl={flx(n)}", f"fl_end={flx(m)}", f"pm={dyn*rm.pth(n)}", f"pm_end={dyn*rm.pth(m)}",
            "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", f"H_end={rm.h0(m)}", "dur=0.9", "ramp_start=0.4", "ramp_dur=0.02",
            "attack=0.003", "yinit=0", f"fscale={fs}", "track=1", "fmax=2400", "fmin=30"]
    rows = [json.loads(l) for l in rm.subprocess.run(args, capture_output=True, text=True).stdout.splitlines()]
    seq = [(x["t"], rm.partial_of(x["f0"], fres, fs) if x["rms"] > 20 and x["clarity"] > 0.8 else 0) for x in rows]
    before = [p for t, p in seq if 0.3 < t < 0.4]
    after = [p for t, p in seq if t > 0.5]
    trans = [p for t, p in seq if 0.4 <= t <= 0.6]
    sil = max([len(list(g)) for k, g in itertools.groupby(trans) if k == 0] or [0]) * 25
    settle = next((t - 0.4 for t, p in seq if t > 0.4 and all(q == m for tt, q in seq if tt >= t)), None)
    return {"before": max(set(before), key=before.count) if before else 0, "after": max(set(after), key=after.count) if after else 0,
            "silence_ms": sil, "settle_s": settle}

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    notes = [dict(n, fscale=reg[(n["written"], n["kind"])]["fscale"]) for n in table["notes"] if n["kind"] == "std" and reg[(n["written"], n["kind"])]["fscale"]]
    out = []; ok = {}
    for note in notes:
        n = note["partial"]
        for dm in (1, 2, -1, -2):
            m = n + dm
            if m < 1 or m > 14: continue
            r = jump(table, note, m); r.update({"written": note["written"], "n": n, "m": m})
            out.append(r)
            good = r["before"] == n and r["after"] == m and r["silence_ms"] <= 50
            ok.setdefault(dm, [0, 0]); ok[dm][0] += good; ok[dm][1] += 1
            if not good: print("FAIL", r, flush=True)
    print("success per jump size:", ok)
    print("settle times (s) median/max:", np.median([r["settle_s"] for r in out if r["settle_s"] is not None]), max(r["settle_s"] or 0 for r in out))
    json.dump(out, open(sys.argv[3], "w"), indent=0)
