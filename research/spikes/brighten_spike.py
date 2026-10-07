"""Spike: within-register Overblow (0 <= o < 0.5): p_m x (1 + 2 o), f_l unchanged (variant 2; variant 1 used f_l x (1 + 0.04 o): 24/35 stayed) on the partial-n setting.
Checks the note stays on partial n and the mouthpiece-pressure centroid / f0 increases monotonically with o."""
import json, sys, numpy as np, soundfile as sf
import regime_map as rm

def render(table, note, o, path):
    st, n = note["state"], note["partial"]; fres = [x["f_hz"] for x in table["states"][st]]; fs = note["fscale"]
    fl = rm.ratio(n) * fres[n - 1] * fs * 1.0; pm = 2.5 * rm.pth(n) * (1 + 2 * o)
    args = [rm.S, rm.poles_file(table, st), f"fl={fl}", f"pm={pm}", "Ql=20", "mu=9", "b=12e-3", f"H={rm.h0(n)}", "dur=0.8",
            "attack=0.003", "yinit=0", f"fscale={fs}", f"wav={path}", "fmax=2400", "fmin=30"]
    r = json.loads(rm.subprocess.run(args, capture_output=True, text=True).stdout.splitlines()[-1])
    x = np.fromfile(path, dtype=np.float32)[int(0.4 * 48000):]
    X = np.abs(np.fft.rfft(x * np.hanning(len(x)))) ** 2; f = np.fft.rfftfreq(len(x), 1 / 48000)
    return rm.partial_of(r["f0"], fres, fs), float(np.sum(f * X) / np.sum(X)) / r["f0"]

if __name__ == "__main__":
    table = json.load(open(sys.argv[1])); reg = {(r["written"], r["kind"]): r for r in json.load(open(sys.argv[2]))}
    notes = [dict(n, fscale=reg[(n["written"], n["kind"])]["fscale"]) for n in table["notes"] if n["kind"] == "std" and reg[(n["written"], n["kind"])]["fscale"]]
    okp = okc = 0
    for note in notes:
        vals = [render(table, note, o, "/tmp/claude-1000/-home-claude-projects-trumpet-vst/f855d006-2b49-5a65-8b7d-22066c71447d/scratchpad/b.raw") for o in (0, 0.15, 0.3, 0.45)]
        stay = all(p == note["partial"] for p, _ in vals); inc = all(b[1] > a[1] for a, b in zip(vals, vals[1:]))
        okp += stay; okc += inc
        print(note["written"], "p%d" % note["partial"], "partials", [p for p, _ in vals], "ncentroid", [round(c, 2) for _, c in vals])
    print("stay on partial: %d/%d, centroid increasing: %d/%d" % (okp, len(notes), okc, len(notes)))
