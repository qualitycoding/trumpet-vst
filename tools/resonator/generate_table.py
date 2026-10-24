# SPDX-License-Identifier: Apache-2.0
"""Generates data/trumpet_resonators.json (format tpt-resonators-1, D-004). Reference: research/spikes/make_table.py
plus the C-099 corrections.

Usage: python -m tools.resonator.generate_table --fingerings tests/fixtures/trumpet_fingerings.txt --out <json>
         [--keep-calibration <existing json>]
Deterministic: same inputs -> identical bytes (json.dumps(indent=1, sort_keys=True), floats rounded to 10 significant
digits, provenance.git = `git rev-parse HEAD` or "unknown"). --keep-calibration copies "fscale" of matching notes from
an existing table (calibration is done by tpt_calibrate, S-009)."""
import argparse
import cmath
import json
import math
import os
import pathlib
import subprocess
import sys

os.environ["OMP_NUM_THREADS"] = "1"
os.environ["OPENBLAS_NUM_THREADS"] = "1"

import numpy as np                        # noqa: E402
from scipy.optimize import least_squares  # noqa: E402

from . import bore_fit, tmm               # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
MEASURED = ROOT / "tests" / "fixtures" / "freour2022_open_modes.txt"
NMODES, F_MAX = 14, 1800.0
F_FIT = np.linspace(40.0, 2000.0, 1961)            # modal fits (1 Hz)
F_PK = np.linspace(40.0, 2000.0, 3921)             # peak picking for the bisections (0.5 Hz)
VALVE_SEMITONES = {"2": 1, "1": 2, "3": 3}
COMBOS = ["0", "1", "2", "3", "12", "13", "23", "123"]
CENTS_REFERENCE = ("12-TET, A4 = 440 Hz, concert = written - 2; natural_dev_cents = hs_dev + dev(state) - dev(open)")


def r10(x):
    """Round to 10 significant digits (D-004 number formatting)."""
    return float(format(float(x), ".10g"))


def round_tree(o):
    if isinstance(o, float):
        return r10(o)
    if isinstance(o, dict):
        return {k: round_tree(v) for k, v in o.items()}
    if isinstance(o, (list, tuple)):
        return [round_tree(v) for v in o]
    return o


def et_hz(concert_midi):
    return 440.0 * 2 ** ((concert_midi - 69) / 12)


def cents(f, ref):
    return 1200 * math.log2(f / ref)


def read_measured():
    rows = [l.split() for l in MEASURED.read_text().splitlines() if l.strip() and not l.startswith("#")]
    return [(complex(float(a), float(b)), complex(float(c), float(d))) for a, b, c, d in rows]


def read_fingerings(path):
    rows = []
    for line in pathlib.Path(path).read_text().splitlines():
        line = line.split("#")[0].strip()
        if not line:
            continue
        w, valves, p, trig, kind = line.split()
        rows.append({"written": int(w), "valves": valves, "partial": int(p.lstrip("p")), "trigger": trig, "kind": kind})
    return rows


class Bore:
    """The fitted bore with its valve loops; impedance and peak/pole evaluation of every state."""

    def __init__(self, geo):
        self.geo = geo
        self.loops = {}

    def zin(self, f, combo, trig_extra=0.0):
        extra = sum(self.loops[v] for v in combo if v != "0") + trig_extra
        z, h = tmm.input_impedance(f, self.geo["profile"], self.geo["valve_x_m"], extra, self.geo["valve_r_m"])
        return z, h

    def peaks(self, combo, trig_extra=0.0):
        z, _ = self.zin(F_PK, combo, trig_extra)
        return tmm.peaks(F_PK, z)[:NMODES]

    def fit(self, combo, trig_extra=0.0):
        z, _ = self.zin(F_FIT, combo, trig_extra)
        return tmm.complex_modal_fit(F_FIT, z, NMODES, F_MAX, tol=1e-10)

    def tune_loops(self):
        open_pk = self.peaks("0")
        for v, st in VALVE_SEMITONES.items():
            lo, hi = 0.0, 0.6
            for _ in range(50):
                mid = 0.5 * (lo + hi)
                self.loops = {**self.loops, v: mid}
                pk = self.peaks(v)
                low = np.mean([cents(open_pk[k][0], pk[k][0]) for k in range(2, 6)])
                lo, hi = (mid, hi) if low < 100 * st else (lo, mid)
            self.loops[v] = 0.5 * (lo + hi)
        return open_pk


def passive_phase(s):
    """Phase of the residue of a passive second-order mode with pole s: atan(|Re s| / Im s) (Z(0) = 0)."""
    return math.atan2(-s.real, s.imag)


def calibration(open_modes):
    """Pole ratios s_meas/s_TMM-open (complex), residue magnitude ratios |R_meas|/|R_TMM-open| and the measured residue
    phase excess over the passive phase, for k = 1..14 (k > 11 reuse the k = 11 ratios and have no phase excess)."""
    meas = read_measured()
    cs, cm, dl = [], [], []
    for k in range(NMODES):
        if k < len(meas):
            cs.append(meas[k][0] / open_modes[k][0])
            cm.append(abs(meas[k][1]) / abs(open_modes[k][1]))
            dl.append(cmath.phase(meas[k][1]) - passive_phase(meas[k][0]))
        else:
            cs.append(cs[-1]); cm.append(cm[-1]); dl.append(0.0)   # modes above the measured ones: passive phase
    return cs, cm, dl


def calibrated(modes, cal):
    """Calibrated modes: s * c_s,k and |R| * |c_R,k| with the phase passive_phase(s') + delta_k. The residue phase is not
    transferred as a ratio from the TMM fit: its fit noise made the 3-valve states active (Re Z < 0 near DC) and the
    reference integrator diverged (DEVIATIONS.md S-005). The open state still equals the measured modes."""
    cs, cm, dl = cal
    out = []
    for k, (s, r) in enumerate(modes):
        s2 = s * cs[k]
        out.append((s2, cmath.rect(abs(r) * cm[k], passive_phase(s2) + dl[k])))
    return out


def hs_dev(n):
    return 1200 * math.log2(n) - 100 * round(12 * math.log2(n))


def smoothed_db(f, mag_db):
    """1/3-octave smoothing: mean of the dB values within +-1/6 octave."""
    out = np.empty_like(mag_db)
    for i, fc in enumerate(f):
        sel = (f >= fc * 2 ** (-1 / 6)) & (f <= fc * 2 ** (1 / 6))
        out[i] = np.mean(mag_db[sel])
    return out


def section_db(kind, f_hz, q, gain_db, f):
    """Magnitude (dB) of the analogue RBJ prototype of a biquad section."""
    s = 1j * f / f_hz
    a = 10 ** (gain_db / 40)
    if kind == "highpass":
        h = s * s / (s * s + s / q + 1)
    elif kind == "lowpass":
        h = 1 / (s * s + s / q + 1)
    elif kind == "peak":
        h = (s * s + s * (a / q) + 1) / (s * s + s / (a * q) + 1)
    elif kind == "highshelf":
        h = a * (a * s * s + math.sqrt(a) / q * s + 1) / (s * s + math.sqrt(a) / q * s + a)
    elif kind == "lowshelf":
        h = a * (s * s + math.sqrt(a) / q * s + a) / (a * s * s + math.sqrt(a) / q * s + 1)
    else:
        raise ValueError(kind)
    return 20 * np.log10(np.abs(h))


def fit_radiation(bore):
    """Radiation sections (D-004): least squares on the 1/3-octave-smoothed log magnitude of |j w H / Z_rad| of the open
    state, 80 Hz - 8 kHz, up to 6 sections, accepted at RMS <= 3 dB (the overall level is free: K_out sets it)."""
    f_all = np.linspace(40.0, 8200.0, 16321)
    z, h = bore.zin(f_all, "0")
    r_end = bore.geo["profile"][-1][1]
    target = 20 * np.log10(np.abs(2j * np.pi * f_all * h / tmm.zrad(f_all, r_end)))
    fg = np.geomspace(80.0, 8000.0, 160)
    t_db = smoothed_db(f_all, target)
    t_db = np.interp(fg, f_all, t_db)

    def model(sections, fr):
        return sum(section_db(k, f_, q, g, fr) for k, f_, q, g in sections)

    def pack(secs):
        return np.array([v for _, f_, q, g in secs for v in (math.log(f_), math.log(q), g)])

    def unpack(x, kinds):
        return [(k, math.exp(x[3 * i]), math.exp(x[3 * i + 1]), x[3 * i + 2]) for i, k in enumerate(kinds)]

    secs = [("highpass", 1000.0, 0.7, 0.0), ("peak", 2000.0, 1.0, 0.0)]
    while True:
        kinds = [s[0] for s in secs]
        lo = np.array([v for k in kinds for v in (math.log(10.0), math.log(0.1), -40.0)])
        hi = np.array([v for k in kinds for v in (math.log(20000.0), math.log(10.0), 40.0)])
        lo[2::3] = np.where(np.array([k in ("peak", "highshelf", "lowshelf") for k in kinds]), -40.0, -1e-9)
        hi[2::3] = np.where(np.array([k in ("peak", "highshelf", "lowshelf") for k in kinds]), 40.0, 1e-9)

        def res(x):
            m = model(unpack(x, kinds), fg)
            return (m - np.mean(m - t_db)) - t_db

        x0 = np.clip(pack(secs), lo + 1e-12, hi - 1e-12)
        fit = least_squares(res, x0, bounds=(lo, hi), ftol=1e-14, xtol=1e-14, gtol=1e-14, max_nfev=2000)
        secs = unpack(fit.x, kinds)
        r = res(fit.x)
        rms = float(np.sqrt(np.mean(r ** 2)))
        if rms <= 3.0 or len(secs) >= 6:
            break
        i = int(np.argmax(np.abs(r)))
        secs.append(("peak", float(fg[i]), 1.0, float(-r[i])))
    sections = [{"type": k, "f_hz": f_, "q": q, "gain_db": (g if k in ("peak", "highshelf", "lowshelf") else 0.0)}
                for k, f_, q, g in secs]
    return sections, rms


def git_head():
    try:
        return subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def build(fingerings, keep=None):
    params, bore_info = bore_fit.load_params()
    bore = Bore(bore_fit.geometry(params))
    open_pk = bore.tune_loops()
    modes = {c: bore.fit(c) for c in COMBOS}
    cal = calibration(modes["0"])
    cs = cal[0]
    states = {c: calibrated(modes[c], cal) for c in COMBOS}
    open_f = [m[0].imag / (2 * math.pi) for m in states["0"]]
    open_dev = {k + 1: cents(open_f[k], et_hz(46 + 12 * math.log2(k + 1))) for k in range(NMODES)}
    # peak-to-pole frequency ratio of the open horn, used to steer the trigger bisections on the cheap peak list
    fr = [open_f[k] / open_pk[k][0] for k in range(NMODES)]

    state_rows, notes, trig_states = [], [], {}
    for c in COMBOS:
        state_rows.append({"id": c, "valves": c, "trigger": "-", "trigger_len_m": 0.0, "modes": states[c]})
    for r in fingerings:
        concert, n, key = r["written"] - 2, r["partial"], r["valves"]
        target = et_hz(concert)
        state_id, trig_len = key, 0.0
        if r["trigger"] != "-":
            v = r["trigger"]
            want = open_dev[n]
            saved = bore.loops[v]

            def dev_peak(x):
                bore.loops[v] = saved
                return cents(bore.peaks(key, x)[n - 1][0] * fr[n - 1], target)

            lo, hi = 0.0, 0.08
            for _ in range(40):
                mid = 0.5 * (lo + hi)
                lo, hi = (mid, hi) if dev_peak(mid) > want else (lo, mid)
            trig_len = 0.5 * (lo + hi)
            # two Newton steps on the pole frequencies of the modal fit (like with like, C-099)
            slope = (dev_peak(trig_len + 1e-4) - dev_peak(trig_len - 1e-4)) / 2e-4
            for _ in range(2):
                m = calibrated(bore.fit(key, trig_len), cal)
                d = cents(m[n - 1][0].imag / (2 * math.pi), target)
                trig_len = min(0.08, max(0.0, trig_len - (d - want) / slope))
            bore.loops[v] = saved
            state_id = f"{key}+t{v}@{r['written']}"
            if state_id not in trig_states:
                m = calibrated(bore.fit(key, trig_len), cal)
                trig_states[state_id] = m
                state_rows.append({"id": state_id, "valves": key, "trigger": v, "trigger_len_m": trig_len, "modes": m})
        st = trig_states[state_id] if state_id in trig_states else states[key]
        dev_state = cents(st[n - 1][0].imag / (2 * math.pi), target)
        dev_open = cents(open_f[n - 1], et_hz(46 + round(12 * math.log2(n))))
        notes.append({"written": r["written"], "kind": r["kind"], "state": state_id, "partial": n, "fscale": 1.0,
                      "natural_dev_cents": hs_dev(n) + dev_state - dev_open})
    if keep:
        old = {(x["written"], x["kind"]): x["fscale"] for x in json.loads(pathlib.Path(keep).read_text())["notes"]}
        for x in notes:
            x["fscale"] = old.get((x["written"], x["kind"]), x["fscale"])

    radiation, rad_rms = fit_radiation(bore)
    out = {
        "format": "tpt-resonators-1",
        "air": {"T_C": 27.0, "c": tmm.C27, "rho": tmm.RHO27},
        "nlp": {"length_m": 0.85, "beta": 1.2},
        "radiation": radiation,
        "states": [{**s, "modes": [{"s": [m[0].real, m[0].imag], "R": [m[1].real, m[1].imag]} for m in s["modes"]]}
                   for s in state_rows],
        "notes": notes,
        "provenance": {
            "generator": "tools/resonator/generate_table.py", "git": git_head(), "cents_reference": CENTS_REFERENCE,
            "bore_params": params, "bore_cents_err": bore_info["cents_err"], "loops_m": bore.loops,
            "radiation_rms_db": rad_rms, "source": "Freour et al. 2022 Table 1 (CC BY 4.0), own parametric bore (C-076)",
        },
    }
    return round_tree(out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--fingerings", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--keep-calibration", metavar="JSON", default=None)
    a = ap.parse_args(argv)
    keep = a.keep_calibration if a.keep_calibration and pathlib.Path(a.keep_calibration).exists() else None
    table = build(read_fingerings(a.fingerings), keep)
    pathlib.Path(a.out).write_text(json.dumps(table, indent=1, sort_keys=True) + "\n")
    print(f"wrote {a.out}: {len(table['states'])} states, {len(table['notes'])} notes, "
          f"radiation {len(table['radiation'])} sections (rms {table['provenance']['radiation_rms_db']:.2f} dB)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
