"""Spike: plane-wave transfer-matrix model (TMM) of a B-flat trumpet bore for the 8 valve combinations.

Bore = list of (x_m, r_m) points (mouthpiece rim at x = 0 ... bell rim). The profile is cut into short conical
frusta; each frustum uses the exact spherical-wave cone matrix (Chaigne & Kergomard 2016, sec. 7.4; Fletcher &
Rossing 1998, eq. 8.24) with visco-thermal losses at the mean radius (Chaigne & Kergomard sec. 5.5 approximation,
as in the clarinet-vst spike). Valve loops are cylinders inserted at x_valve. Radiation: unflanged pipe (Levine &
Schwinger low-frequency fit) at the bell rim.

Outputs per combination: Zin(f) / Zc (normalised by the input characteristic impedance), the pressure transfer
function H(f) = p_bell_rim / p_input, peak list (f, |Z|, Q), and a modal fit (second-order modes).
"""
import json, sys
import numpy as np

C0, RHO = 343.37, 1.2041          # 20 degC (clarinet-vst spike values)
LV = 4.0e-8                         # visco-thermal boundary length scale (m), Chaigne & Kergomard
COMBOS = ["0", "2", "1", "12", "23", "13", "123", "3"]   # display order: lowering 0,1,2,3,4,5,6,3 semitones


def gamma(f, r):
    s = 2j * np.pi * f
    return s / C0 + 1.044 / r * np.sqrt(2 * LV / C0) * np.sqrt(s) + 1.080 * LV / r ** 2


def zc(r):
    return RHO * C0 / (np.pi * r ** 2)


def cyl(f, r, L):
    G = gamma(f, r)
    Z = zc(r)
    ch, sh = np.cosh(G * L), np.sinh(G * L)
    return np.array([[ch, Z * sh], [sh / Z, ch]])


def cone(f, r1, r2, L):
    """Lossy conical frustum (input radius r1, output r2). Falls back to a cylinder when |r2-r1| is tiny."""
    if abs(r2 - r1) < 1e-7:
        return cyl(f, r1, L)
    rm = 0.5 * (r1 + r2)
    G = gamma(f, rm)                       # propagation constant with losses at the mean radius
    x1 = r1 * L / (r2 - r1)               # signed distances from the cone apex
    x2 = x1 + L

    def fund(x, r):
        # Spherical-wave basis p = e^{-+Gx}/x; U = -S(x) dp/dx / (G rho c)  (series impedance G*rho*c/S)
        ea, eb = np.exp(-G * x), np.exp(G * x)
        k = -np.pi * r ** 2 / (G * RHO * C0)
        return (ea / x, eb / x, k * (-(G * x + 1) * ea / x ** 2), k * ((G * x - 1) * eb / x ** 2))

    a1, b1, c1, d1 = fund(x1, r1)
    a2, b2, c2, d2 = fund(x2, r2)
    det = a2 * d2 - b2 * c2                # M = F(x1) F(x2)^-1 maps (p,U) at the output to the input
    i11, i12, i21, i22 = d2 / det, -b2 / det, -c2 / det, a2 / det
    return np.array([[a1 * i11 + b1 * i21, a1 * i12 + b1 * i22],
                     [c1 * i11 + d1 * i21, c1 * i12 + d1 * i22]])


def zrad(f, r):
    k = 2 * np.pi * f / C0
    return zc(r) * (0.25 * (k * r) ** 2 + 1j * 0.6133 * k * r)


def segments(profile, valve_x, extra_len, r_valve):
    """Return list of ('cone', r1, r2, L) with a cylinder of extra_len inserted at valve_x."""
    segs = []
    for (x1, r1), (x2, r2) in zip(profile[:-1], profile[1:]):
        if extra_len > 0 and x1 <= valve_x < x2:
            t = (valve_x - x1) / (x2 - x1)
            rv = r1 + t * (r2 - r1)
            if valve_x > x1:
                segs.append(("cone", r1, rv, valve_x - x1))
            segs.append(("cone", r_valve, r_valve, extra_len))
            segs.append(("cone", rv, r2, x2 - valve_x))
        else:
            segs.append(("cone", r1, r2, x2 - x1))
    return segs


def chain(f, segs):
    M = np.broadcast_to(np.eye(2, dtype=complex)[:, :, None], (2, 2, len(f))).copy()
    for _, r1, r2, L in segs:
        if L <= 0:
            continue
        T = cone(f, r1, r2, L)
        M = np.einsum("ijf,jkf->ikf", M, T)
    return M


def zin_and_h(f, segs):
    M = chain(f, segs)
    r_end = segs[-1][2]
    ZL = zrad(f, r_end)
    Zin = (M[0, 0] * ZL + M[0, 1]) / (M[1, 0] * ZL + M[1, 1])
    # p_in = A p_out + B u_out, u_out = p_out / ZL  ->  H = p_out / p_in
    H = 1.0 / (M[0, 0] + M[0, 1] / ZL)
    return Zin, H


def peaks(f, z):
    m = np.abs(z)
    out = []
    for i in range(1, len(m) - 1):
        if m[i] > m[i - 1] and m[i] >= m[i + 1]:
            # half-power bandwidth
            half = m[i] / np.sqrt(2)
            lo = i
            while lo > 0 and m[lo] > half:
                lo -= 1
            hi = i
            while hi < len(m) - 1 and m[hi] > half:
                hi += 1
            bw = f[hi] - f[lo]
            # parabolic refinement of the peak frequency
            y0, y1, y2 = m[i - 1], m[i], m[i + 1]
            den = y0 - 2 * y1 + y2
            dx = 0.5 * (y0 - y2) / den if den != 0 else 0.0
            fp = f[i] + dx * (f[1] - f[0])
            out.append((float(fp), float(m[i]), float(fp / bw) if bw > 0 else float("nan")))
    return out


def modal_fit(f, z, pk, fmax):
    """Second-order modes Z_n(w) = A_n * j w w_n/Q_n / (w_n^2 - w^2 + j w w_n/Q_n), A_n = |Z| at the peak.
    Refined by linear least squares on the A_n (frequencies and Q fixed from the peak list)."""
    use = [p for p in pk if p[0] <= fmax]
    w = 2 * np.pi * f
    basis = []
    for fp, _, Q in use:
        wn = 2 * np.pi * fp
        basis.append(1j * w * wn / Q / (wn ** 2 - w ** 2 + 1j * w * wn / Q))
    B = np.array(basis).T
    sel = f <= fmax * 1.1
    Bs = np.vstack([B[sel].real, B[sel].imag])
    zs = np.concatenate([z[sel].real, z[sel].imag])
    A, *_ = np.linalg.lstsq(Bs, zs, rcond=None)
    zfit = B @ A
    err = np.linalg.norm(zfit[sel] - z[sel]) / np.linalg.norm(z[sel])
    modes = [{"f": round(fp, 3), "Q": round(Q, 3), "A": round(float(a), 4)} for (fp, _, Q), a in zip(use, A)]
    return modes, float(err)


def run(geometry, fmax_fit=2000.0, verbose=True):
    g = geometry
    f = np.linspace(20, 3000, 29801)
    out = {}
    for combo in COMBOS:
        extra = sum(g["valve_loops_m"][v] for v in combo if v != "0")
        segs = segments(g["profile"], g["valve_x_m"], extra, g["valve_r_m"])
        Zin, H = zin_and_h(f, segs)
        z = Zin / zc(g["profile"][0][1])
        pk = peaks(f, z)
        modes, err = modal_fit(f, z, pk, fmax_fit)
        out[combo] = {"peaks": pk[:16], "modes": modes, "fit_rel_err": err,
                      "H_db": [float(20 * np.log10(abs(H[np.argmin(abs(f - fq))]))) for fq in (100, 250, 500, 1000, 1500, 2000, 3000)]}
        if verbose:
            print(combo, "peaks Hz:", [round(p[0], 1) for p in pk[:12]], "fit err %.3f" % err)
    return out


if __name__ == "__main__":
    geo = json.load(open(sys.argv[1]))
    res = run(geo)
    if len(sys.argv) > 2:
        json.dump(res, open(sys.argv[2], "w"), indent=1)
