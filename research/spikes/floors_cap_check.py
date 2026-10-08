"""Round 4 check of the corrections from the R5 review: (a) extended-range blowing floors (D-023: written 86, 87, 89
>= mf factor 2.5, written 88 >= ff factor 5.0) sustain for 3 s at a requested pp; (b) Overblow capped at partial
max(n, 9) (D-009): p8 notes jumping to p9 sustain for 2.5 s after the jump. Sustain gate: rms(last 0.3 s) >=
0.8 x rms(0.5-0.8 s) and the sounding partial over the last 0.3 s equals the target."""
import json, sys, numpy as np, regime_map as rm, overblow_d as od

FLOOR = {86: 2.5, 87: 2.5, 88: 5.0, 89: 2.5}

def sustained(rows, fres, fs, target, t0=0.5, t1=0.8):
    a = [x["rms"] for x in rows if t0 <= x["t"] <= t1]; b = [x["rms"] for x in rows if x["t"] >= rows[-1]["t"] - 0.3]
    parts = [rm.partial_of(x["f0"], fres, fs) for x in rows if x["t"] >= rows[-1]["t"] - 0.3 and x["rms"] > 20]
    return bool(a and b and np.median(b) >= 0.8 * np.median(a) and parts and max(set(parts), key=parts.count) == target)

table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
ok = 0; tot = 0
for w, k in FLOOR.items():
    note = next(x for x in table["notes"] if x["written"] == w and x["kind"] == "std"); fs = reg[(w, "std")]["fscale"] or 1.0
    rows, fres = rm.sim(table, note["state"], note["partial"], max(1.3, k) * rm.pth(note["partial"]), fs, dur=3.0, track=True)
    s = sustained(rows, fres, fs, note["partial"]); ok += s; tot += 1
    print("floor", w, "factor", k, "sustains", s, flush=True)
for note in table["notes"]:
    if note["kind"] != "std" or note["partial"] != 8: continue
    fs = reg[(note["written"], "std")]["fscale"]; n = note["partial"]; m = 9
    st = note["state"]; fres = [x["f_hz"] for x in table["states"][st]]
    flx = lambda q: rm.ratio(q) * fres[q - 1] * fs
    args = [rm.S, rm.poles_file(table, st), f"fl={flx(n)}", f"fl_end={flx(m)}", f"pm={2.5*rm.pth(n)}", f"pm_end={2.5*rm.pth(m)}",
            "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", f"H_end={rm.h0(m)}", "dur=3.0", "ramp_start=0.4", "ramp_dur=0.02",
            "attack=0.003", "yinit=0", f"fscale={fs}", "track=1", "fmax=2400", "fmin=30"]
    rows = [json.loads(l) for l in rm.subprocess.run(args, capture_output=True, text=True).stdout.splitlines()]
    s = sustained(rows, fres, fs, m, 0.6, 0.9); ok += s; tot += 1
    print("jump", note["written"], f"p{n}->p{m}", "sustains", s, flush=True)
print(f"{ok}/{tot} sustained")
