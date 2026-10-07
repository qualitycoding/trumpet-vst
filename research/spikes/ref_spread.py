"""Spike Q-R2: spread of realism metrics in the TinySOL "Trumpet in C" ordinario recordings, and feasibility of
candidate thresholds (lesson L-20261007T150400Z: show that a plausible prediction passes before freezing).

Usage: python ref_spread.py <TinySOL root containing Brass/Trumpet_C/ordinario> <out.json>

Metrics per note (steady segment = the longest run of 10 ms frames within 6 dB of the note's maximum, starting
>= 0.15 s after onset, at most 1.0 s long):
  harm_db[h]     levels of harmonics 1..10 in dB relative to the strongest of them (Hann FFT, peak within +-3 % of h f0)
  centroid_hz    spectral centroid of the power spectrum 20 Hz .. 10 kHz
  ncentroid      centroid / f0 (pitch-normalised brightness)
  onset_ms       time from the first 1 ms frame above -40 dB re max to the first frame >= 50 % (-6 dB) of the
                 maximum reached within the first 300 ms after onset ("onset-based", ignores later swells)
Feasibility ("neighbour prediction"): for each note, predict its metrics by the mean of the same-dynamic notes at
pitch +-1 and +-2 semitones (excluding the note itself) and measure the error as a synthetic model would be measured.
"""
import glob, json, os, re, sys
import numpy as np
import soundfile as sf

NAME = re.compile(r"TpC-ord-([A-G]#?)(\d)-(pp|mf|ff)-")
PC = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6, "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}


def midi_of(name, octave):
    return 12 * (int(octave) + 1) + PC[name]


def frames_db(x, fs, win):
    n = int(win * fs)
    m = len(x) // n
    e = np.sqrt(np.mean(x[: m * n].reshape(m, n) ** 2, axis=1)) + 1e-12
    return 20 * np.log10(e / e.max()), n


def onset_ms(x, fs):
    db, n = frames_db(x, fs, 0.001)
    i0 = int(np.argmax(db > -40))
    seg = db[i0: i0 + 300]
    peak = seg.max()
    i1 = int(np.argmax(seg >= peak - 6.0206))
    return float(i1)  # 1 ms frames


def steady(x, fs):
    db, n = frames_db(x, fs, 0.01)
    i0 = int(np.argmax(db > -40)) + 15
    ok = db >= -6
    best, cur, start, bstart = 0, 0, i0, i0
    for i in range(i0, len(db)):
        if ok[i]:
            if cur == 0:
                start = i
            cur += 1
            if cur > best:
                best, bstart = cur, start
        else:
            cur = 0
    best = min(best, 100)
    return x[bstart * n: (bstart + best) * n]


def harmonics(seg, fs, f0, nh=10):
    N = 1 << int(np.ceil(np.log2(4 * len(seg))))
    X = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), N))
    fr = np.fft.rfftfreq(N, 1 / fs)
    # refine f0 from the strongest of the first 3 harmonics
    lv = []
    for h in range(1, nh + 1):
        lo, hi = np.searchsorted(fr, [h * f0 * 0.97, h * f0 * 1.03])
        lv.append(X[lo:hi].max() if hi > lo else 1e-12)
    lv = 20 * np.log10(np.array(lv) + 1e-12)
    P = X ** 2
    sel = (fr >= 20) & (fr <= 10000)
    cen = float(np.sum(fr[sel] * P[sel]) / np.sum(P[sel]))
    return lv - lv.max(), cen


def analyse(root):
    rows = []
    for p in sorted(glob.glob(os.path.join(root, "**", "TpC-ord-*.wav"), recursive=True)):
        m = NAME.search(os.path.basename(p))
        if not m:
            continue
        x, fs = sf.read(p)
        midi = midi_of(m.group(1), m.group(2))
        f0 = 440.0 * 2 ** ((midi - 69) / 12)
        seg = steady(x, fs)
        hdb, cen = harmonics(seg, fs, f0)
        rows.append({"file": os.path.basename(p), "midi": midi, "dyn": m.group(3), "f0": f0,
                     "harm_db": [round(float(v), 2) for v in hdb], "centroid_hz": round(cen, 1),
                     "ncentroid": round(cen / f0, 3), "onset_ms": onset_ms(x, fs),
                     "steady_s": round(len(seg) / fs, 3)})
    return rows


def neighbour_eval(rows, lo=54, hi=82, nh=8):
    by = {(r["midi"], r["dyn"]): r for r in rows}
    res = {"pp": [], "mf": [], "ff": []}
    for (midi, dyn), r in by.items():
        if not (lo <= midi <= hi):
            continue
        nb = [by[(midi + d, dyn)] for d in (-2, -1, 1, 2) if (midi + d, dyn) in by]
        if len(nb) < 2:
            continue
        ph = np.mean([n["harm_db"][:nh] for n in nb], axis=0)
        pc = np.exp(np.mean([np.log(n["ncentroid"]) for n in nb])) * r["f0"]
        po = float(np.median([n["onset_ms"] for n in nb]))
        mad = float(np.mean(np.abs(ph - np.array(r["harm_db"][:nh]))))
        res[dyn].append({"midi": midi, "harm_mad_db": mad, "centroid_ratio": pc / r["centroid_hz"],
                         "onset_ratio": (po + 1) / (r["onset_ms"] + 1)})
    return res


def summarise(rows, ev):
    out = {"n_notes": len(rows), "per_dynamic": {}}
    for dyn in ("pp", "mf", "ff"):
        rs = [r for r in rows if r["dyn"] == dyn and 54 <= r["midi"] <= 82]
        e = ev[dyn]
        mads = np.array([x["harm_mad_db"] for x in e])
        cr = np.array([x["centroid_ratio"] for x in e])
        orr = np.array([x["onset_ratio"] for x in e])
        on = np.array([r["onset_ms"] for r in rs])
        out["per_dynamic"][dyn] = {
            "n": len(rs),
            "onset_ms_q10_med_q90": [float(np.percentile(on, q)) for q in (10, 50, 90)],
            "ncentroid_q10_med_q90": [float(np.percentile([r["ncentroid"] for r in rs], q)) for q in (10, 50, 90)],
            "neighbour_harm_mad_mean_db": float(mads.mean()),
            "neighbour_harm_mad_q90_db": float(np.percentile(mads, 90)),
            "neighbour_harm_mad_max_db": float(mads.max()),
            "neighbour_centroid_ratio_q05_q95": [float(np.percentile(cr, 5)), float(np.percentile(cr, 95))],
            "neighbour_centroid_in_0.8_1.25": float(np.mean((cr >= 0.8) & (cr <= 1.25))),
            "neighbour_centroid_in_0.75_1.33": float(np.mean((cr >= 0.75) & (cr <= 1.333))),
            "neighbour_onset_ratio_in_0.5_2": float(np.mean((orr >= 0.5) & (orr <= 2))),
            "neighbour_onset_ratio_in_0.33_3": float(np.mean((orr >= 1 / 3) & (orr <= 3))),
            "neighbour_onset_ratio_q05_q95": [float(np.percentile(orr, 5)), float(np.percentile(orr, 95))],
        }
    # dynamics ordering of brightness per pitch
    by = {(r["midi"], r["dyn"]): r for r in rows}
    pitches = sorted({r["midi"] for r in rows if 54 <= r["midi"] <= 82})
    ordered = [m for m in pitches if all((m, d) in by for d in ("pp", "mf", "ff"))]
    inc = [m for m in ordered if by[(m, "pp")]["ncentroid"] < by[(m, "mf")]["ncentroid"] < by[(m, "ff")]["ncentroid"]]
    inc2 = [m for m in ordered if by[(m, "pp")]["ncentroid"] < by[(m, "ff")]["ncentroid"]]
    out["dyn_order_strict_fraction"] = len(inc) / len(ordered)
    out["dyn_order_pp_lt_ff_fraction"] = len(inc2) / len(ordered)
    # brassiness: high-harmonic (6..10) mean level re strongest, ff minus pp, per pitch
    br = [float(np.mean(by[(m, "ff")]["harm_db"][5:10]) - np.mean(by[(m, "pp")]["harm_db"][5:10])) for m in ordered]
    out["brass_ff_minus_pp_h6_10_db_q10_med_q90"] = [float(np.percentile(br, q)) for q in (10, 50, 90)]
    out["brass_ff_minus_pp_positive_fraction"] = float(np.mean(np.array(br) > 0))
    return out


if __name__ == "__main__":
    rows = analyse(sys.argv[1])
    ev = neighbour_eval(rows)
    s = summarise(rows, ev)
    json.dump({"summary": s, "rows": rows}, open(sys.argv[2], "w"), indent=1)
    print(json.dumps(s, indent=1))
