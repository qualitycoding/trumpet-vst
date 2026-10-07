"""Spike: fit a parametric B-flat trumpet bore (our own geometry, no third-party geometry data) so that its open-valve
TMM resonance frequencies match the measured trumpet of Freour et al. 2022, Table 1 (modes 2..11), then compute the
8 valve combinations with valve loops inserted in the cylindrical section.

Usage: python bore_fit.py <out.json>
Air at 27 degC (the measured impedance was corrected to 27 degC, Freour 2022 p. 4).
"""
import json, sys
import numpy as np
from scipy.optimize import least_squares
import tmm_trumpet as t

T_C = 27.0
t.C0 = 331.3 * np.sqrt(1 + T_C / 273.15)          # 347.2 m/s
t.RHO = 1.2041 * 293.15 / (273.15 + T_C)          # ideal gas, same pressure

# Freour et al. 2022 Table 1: Im(s_k)/(2 pi), Hz
MEAS = [83.153, 232.70, 348.07, 462.60, 582.14, 690.58, 800.40, 908.04, 1028.07, 1147.65, 1262.27]
R_CUP, R_THROAT, R_CYL, R_END = 8.25e-3, 1.83e-3, 5.84e-3, 61.5e-3   # 0.460" bore, 123 mm bell (typical)


def geometry(p):
    """p = [L_cup, L_backbore, r_backbore_end, L_leadpipe, L_cyl, L_bell, eps]."""
    L_cup, L_bb, r_bb, L_lp, L_cyl, L_b, eps = p
    x = 0.0
    prof = [(0.0, R_CUP)]
    x += 0.4 * L_cup; prof.append((x, R_CUP))                       # cup (cylindrical part)
    x += 0.6 * L_cup; prof.append((x, R_THROAT))                    # cup cone to throat
    x += 0.007; prof.append((x, R_THROAT))                           # throat
    x += L_bb; prof.append((x, r_bb))                               # backbore
    for k in range(1, 9):                                            # leadpipe cone (r_bb -> R_CYL)
        prof.append((x + L_lp * k / 8, r_bb + (R_CYL - r_bb) * k / 8))
    x += L_lp
    x_valve = x + 0.35 * L_cyl
    x += L_cyl; prof.append((x, R_CYL))
    d0 = (R_CYL / R_END) ** (1 / eps) * L_b / (1 - (R_CYL / R_END) ** (1 / eps))
    for xx in np.linspace(0, L_b, 80)[1:]:                           # Bessel bell
        prof.append((x + xx, R_END * (d0 / (L_b - xx + d0)) ** eps))
    return {"profile": prof, "valve_x_m": x_valve, "valve_r_m": R_CYL}


F = np.linspace(40, 1700, 6641)


def peaks_of(geo, extra=0.0):
    segs = t.segments(geo["profile"], geo["valve_x_m"], extra, geo["valve_r_m"])
    Z, H = t.zin_and_h(F, segs)
    z = Z / t.zc(R_CUP)
    return t.peaks(F, z), Z, H


def resid(p):
    pk, _, _ = peaks_of(geometry(p))
    fs = [x[0] for x in pk]
    if len(fs) < 11:
        return np.full(10, 500.0)
    return np.array([1200 * np.log2(fs[i] / MEAS[i]) for i in range(1, 11)])


P0 = [0.010, 0.065, 4.4e-3, 0.24, 0.55, 0.50, 0.75]
LO = [0.005, 0.040, 3.6e-3, 0.15, 0.30, 0.35, 0.45]
HI = [0.020, 0.090, 5.2e-3, 0.35, 0.75, 0.65, 1.10]

if __name__ == "__main__":
    fit = least_squares(resid, P0, bounds=(LO, HI), diff_step=1e-3, max_nfev=200)
    p = fit.x
    r = resid(p)
    geo = geometry(p)
    print("params", np.round(p, 5).tolist())
    print("cents error modes 2..11:", np.round(r, 1).tolist(), "rms %.1f max %.1f" % (np.sqrt(np.mean(r ** 2)), np.abs(r).max()))
    pk0, _, _ = peaks_of(geo)
    print("open peaks:", [round(x[0], 1) for x in pk0[:14]])
    L_total = geo["profile"][-1][0]
    json.dump({"params": p.tolist(), "cents_err": r.tolist(), "geometry": geo, "L_total": L_total,
               "air": {"T_C": T_C, "c": t.C0, "rho": t.RHO}}, open(sys.argv[1], "w"), indent=1)
