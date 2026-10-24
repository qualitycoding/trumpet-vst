# SPDX-License-Identifier: Apache-2.0
"""Fit of the parametric bore to Freour 2022 Table 1 (D-004). Reference: research/spikes/bore_fit.py. The pole
frequencies of a complex modal fit of the TMM impedance are compared with the measured poles (C-099)."""
import json
import pathlib

import numpy as np
from scipy.optimize import least_squares
from . import tmm

PARAMS_FILE = pathlib.Path(__file__).with_name("bore_params.json")   # written by `python -m tools.resonator.bore_fit --write`
T_C = 27.0
C_AIR, RHO_AIR = tmm.C27, tmm.RHO27

# Freour et al. 2022 Table 1: Im(s_k)/(2 pi), Hz
MEAS = [83.153, 232.70, 348.07, 462.60, 582.14, 690.58, 800.40, 908.04, 1028.07, 1147.65, 1262.27]
R_CUP, R_THROAT, R_CYL, R_END = 8.25e-3, 1.83e-3, 5.84e-3, 61.5e-3   # 0.460" bore, 123 mm bell (typical)
N_MODES, F_MAX = 14, 1800.0
F_GRID = np.linspace(40.0, 2000.0, 1961)           # 1 Hz

P0 = [0.010, 0.065, 4.4e-3, 0.24, 0.55, 0.50, 0.75]
LO = [0.005, 0.040, 3.6e-3, 0.15, 0.30, 0.35, 0.45]
HI = [0.020, 0.090, 5.2e-3, 0.35, 0.75, 0.65, 1.10]


def geometry(p):
    """p = [L_cup, L_backbore, r_backbore_end, L_leadpipe, L_cyl, L_bell, eps]."""
    l_cup, l_bb, r_bb, l_lp, l_cyl, l_b, eps = p
    x = 0.0
    prof = [(0.0, R_CUP)]
    x += 0.4 * l_cup; prof.append((x, R_CUP))                        # cup (cylindrical part)
    x += 0.6 * l_cup; prof.append((x, R_THROAT))                     # cup cone to throat
    x += 0.007; prof.append((x, R_THROAT))                            # throat
    x += l_bb; prof.append((x, r_bb))                                # backbore
    for k in range(1, 9):                                             # leadpipe cone (r_bb -> R_CYL)
        prof.append((x + l_lp * k / 8, r_bb + (R_CYL - r_bb) * k / 8))
    x += l_lp
    x_valve = x + 0.35 * l_cyl
    x += l_cyl; prof.append((x, R_CYL))
    d0 = (R_CYL / R_END) ** (1 / eps) * l_b / (1 - (R_CYL / R_END) ** (1 / eps))
    for xx in np.linspace(0, l_b, 80)[1:]:                            # Bessel bell
        prof.append((x + xx, R_END * (d0 / (l_b - xx + d0)) ** eps))
    return {"profile": prof, "valve_x_m": x_valve, "valve_r_m": R_CYL}


def open_poles(geo, f=F_GRID):
    """Poles s_k (rad/s) of the complex modal fit of the open bore's input impedance."""
    z, _ = tmm.input_impedance(f, geo["profile"], geo["valve_x_m"], 0.0, geo["valve_r_m"], c=C_AIR, rho=RHO_AIR)
    return tmm.complex_modal_fit(f, z, N_MODES, F_MAX, tol=1e-8)


def _cents(geo):
    fs = [m[0].imag / (2 * np.pi) for m in open_poles(geo)]
    return np.array([1200 * np.log2(fs[i] / MEAS[i]) for i in range(1, 11)])


def _resid(p):
    try:
        return _cents(geometry(p))
    except (ValueError, AssertionError, np.linalg.LinAlgError, FloatingPointError):
        return np.full(10, 500.0)


def fit():
    """Returns {"params": [...7 floats...], "geometry": {"profile", "valve_x_m", "valve_r_m"}, "cents_err": [10 floats: modes 2..11],
    "air": {"T_C", "c", "rho"}} with the pole frequencies of a complex modal fit compared like with like (C-099).
    Starts from the stored parameters when `bore_params.json` exists, else from P0."""
    start = P0
    if PARAMS_FILE.exists():
        start = json.loads(PARAMS_FILE.read_text())["params"]
    res = least_squares(_resid, start, bounds=(LO, HI), diff_step=1e-3, max_nfev=200)
    p = res.x
    return {"params": p.tolist(), "geometry": geometry(p), "cents_err": _resid(p).tolist(),
            "air": {"T_C": T_C, "c": C_AIR, "rho": RHO_AIR}}


def load_params():
    """(params, info) from `bore_params.json` (one evaluation re-checks the cents errors); fits when the file is missing."""
    if not PARAMS_FILE.exists():
        r = fit()
        return r["params"], {"cents_err": r["cents_err"]}
    p = json.loads(PARAMS_FILE.read_text())["params"]
    return p, {"cents_err": _resid(p).tolist()}


if __name__ == "__main__":
    import sys
    import time
    t0 = time.time()
    r = fit()
    print("params", np.round(r["params"], 5).tolist())
    print("cents error modes 2..11:", np.round(r["cents_err"], 1).tolist(), "max %.2f" % max(abs(c) for c in r["cents_err"]))
    print("seconds %.0f" % (time.time() - t0))
    if "--write" in sys.argv:
        PARAMS_FILE.write_text(json.dumps({"params": [float(format(v, ".10g")) for v in r["params"]],
                                           "cents_err": [round(c, 3) for c in r["cents_err"]]}, indent=1) + "\n")
