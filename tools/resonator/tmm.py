# SPDX-License-Identifier: Apache-2.0
"""Plane-wave transfer-matrix model of the trumpet bore (D-004). Reference implementation:
research/spikes/tmm_trumpet.py (cone matrix verified by the R5 review)."""
import numpy as np
from scipy.optimize import least_squares

C27 = 331.3 * (1 + 27.0 / 273.15) ** 0.5     # m/s at 27 degC
RHO27 = 1.2041 * 293.15 / (273.15 + 27.0)    # kg/m^3 at 27 degC
LV = 4.0e-8                                   # visco-thermal boundary length scale (m), Chaigne & Kergomard


def _gamma(f, r, c, lossless):
    s = 2j * np.pi * f
    if lossless:
        return s / c
    return s / c + 1.044 / r * np.sqrt(2 * LV / c) * np.sqrt(s) + 1.080 * LV / r ** 2


def zc(r, c=C27, rho=RHO27):
    """Characteristic impedance of a cylinder of radius r."""
    return rho * c / (np.pi * r ** 2)


def _cyl(f, r, length, lossless, c, rho):
    g = _gamma(f, r, c, lossless)
    z = zc(r, c, rho)
    ch, sh = np.cosh(g * length), np.sinh(g * length)
    return np.array([[ch, z * sh], [sh / z, ch]])


def cone_matrix(f, r1, r2, length, lossless=False, c=C27, rho=RHO27):
    """2x2xN transfer matrix (p, U) of a conical frustum (cylinder when |r2 - r1| < 1e-7; identity when length < 1e-8 m,
    where the spherical-wave form is singular)."""
    f = np.atleast_1d(np.asarray(f, dtype=float))
    if length < 1e-8:
        return np.broadcast_to(np.eye(2, dtype=complex)[:, :, None], (2, 2, len(f))).copy()
    if abs(r2 - r1) < 1e-7:
        return _cyl(f, r1, length, lossless, c, rho)
    rm = 0.5 * (r1 + r2)
    g = _gamma(f, rm, c, lossless)             # propagation constant (losses at the mean radius)
    x1 = r1 * length / (r2 - r1)               # signed distances from the cone apex
    x2 = x1 + length

    def fund(x, r):
        # spherical-wave basis p = e^{-+Gx}/x; U = -S(x) dp/dx / (G rho c)
        ea, eb = np.exp(-g * x), np.exp(g * x)
        k = -np.pi * r ** 2 / (g * rho * c)
        return ea / x, eb / x, k * (-(g * x + 1) * ea / x ** 2), k * ((g * x - 1) * eb / x ** 2)

    a1, b1, c1, d1 = fund(x1, r1)
    a2, b2, c2, d2 = fund(x2, r2)
    det = a2 * d2 - b2 * c2                    # M = F(x1) F(x2)^-1 maps (p,U) at the output to the input
    i11, i12, i21, i22 = d2 / det, -b2 / det, -c2 / det, a2 / det
    return np.array([[a1 * i11 + b1 * i21, a1 * i12 + b1 * i22],
                     [c1 * i11 + d1 * i21, c1 * i12 + d1 * i22]])


def zrad(f, r, c=C27, rho=RHO27):
    """Unflanged-pipe radiation impedance, low-frequency (Levine-Schwinger) form."""
    k = 2 * np.pi * f / c
    return zc(r, c, rho) * (0.25 * (k * r) ** 2 + 1j * 0.6133 * k * r)


def segments(profile, valve_x, extra_length, valve_r):
    """List of (r1, r2, length) frusta of the profile, with a cylinder of extra_length inserted at valve_x."""
    segs = []
    for (x1, r1), (x2, r2) in zip(profile[:-1], profile[1:]):
        if extra_length > 0 and x1 <= valve_x < x2:
            t = (valve_x - x1) / (x2 - x1)
            rv = r1 + t * (r2 - r1)
            if valve_x > x1:
                segs.append((r1, rv, valve_x - x1))
            segs.append((valve_r, valve_r, extra_length))
            segs.append((rv, r2, x2 - valve_x))
        else:
            segs.append((r1, r2, x2 - x1))
    return segs


def input_impedance(f, profile, valve_x, extra_length, valve_r, lossless=False, c=C27, rho=RHO27):
    """(Zin, H) of the bore: profile = [(x, r), ...]; a cylinder of extra_length and radius valve_r inserted at valve_x;
    unflanged radiation at the last point; H = p_bell / p_in. Zin is in Pa s / m^3."""
    f = np.asarray(f, dtype=float)
    m = np.broadcast_to(np.eye(2, dtype=complex)[:, :, None], (2, 2, len(f))).copy()
    for r1, r2, length in segments(profile, valve_x, extra_length, valve_r):
        if length <= 0:
            continue
        m = np.einsum("ijf,jkf->ikf", m, cone_matrix(f, r1, r2, length, lossless, c, rho))
    zl = zrad(f, profile[-1][1], c, rho)
    zin = (m[0, 0] * zl + m[0, 1]) / (m[1, 0] * zl + m[1, 1])
    h = 1.0 / (m[0, 0] + m[0, 1] / zl)           # p_in = A p_out + B u_out, u_out = p_out / ZL
    return zin, h


def peaks(f, z):
    """List of (f_peak, |z|_peak, Q) for every local maximum of |z| (half-power Q, parabolic refinement)."""
    m = np.abs(z)
    out = []
    for i in np.nonzero((m[1:-1] > m[:-2]) & (m[1:-1] >= m[2:]))[0] + 1:
        half = m[i] / np.sqrt(2)
        lo = i
        while lo > 0 and m[lo] > half:
            lo -= 1
        hi = i
        while hi < len(m) - 1 and m[hi] > half:
            hi += 1
        bw = f[hi] - f[lo]
        y0, y1, y2 = m[i - 1], m[i], m[i + 1]
        den = y0 - 2 * y1 + y2
        dx = 0.5 * (y0 - y2) / den if den != 0 else 0.0
        fp = f[i] + dx * (f[1] - f[0])
        out.append((float(fp), float(m[i]), float(fp / bw) if bw > 0 else float("nan")))
    return out


# --- complex modal fit -------------------------------------------------------------------------------------------------
# Scaled parameters per mode: [sigma / 10, omega / 1000, Re R / 1e9, Im R / 1e9], s = sigma + j omega. Bounds keep every
# pole near its initial (peak) frequency, so mode k stays the k-th resonance: +-4 % for well-defined peaks (Q > 5), +-15 %
# for the heavily damped upper modes.
_SC = np.array([10.0, 1000.0, 1e9, 1e9])


def _model(w, x):
    """Z(w) = sum_k R_k / (jw - s_k) + conj(R_k) / (jw - conj(s_k)); also returns the intermediate terms."""
    p = x.reshape(-1, 4) * _SC
    s = p[:, 0] + 1j * p[:, 1]
    r = p[:, 2] + 1j * p[:, 3]
    jw = 1j * w[:, None]
    d1, d2 = 1.0 / (jw - s[None, :]), 1.0 / (jw - np.conj(s)[None, :])
    z = np.sum(r[None, :] * d1 + np.conj(r)[None, :] * d2, axis=1)
    return z, (s, r, d1, d2)


def _jac(w, x, weight):
    _, (s, r, d1, d2) = _model(w, x)
    cols = [r[None, :] * d1 ** 2 + np.conj(r)[None, :] * d2 ** 2,                       # d/d sigma
            1j * (r[None, :] * d1 ** 2 - np.conj(r)[None, :] * d2 ** 2),                # d/d omega
            d1 + d2,                                                                      # d/d Re R
            1j * (d1 - d2)]                                                               # d/d Im R
    n, k = d1.shape
    j = np.empty((2 * n, 4 * k))
    for i, col in enumerate(cols):
        col = col * (_SC[i] * weight[:, None])
        j[:n, i::4] = col.real
        j[n:, i::4] = col.imag
    return j


def complex_modal_fit(f, z, n_modes, f_max, tol=1e-12, max_nfev=300):
    """Fit Z(w) = sum_k R_k/(jw - s_k) + conj(R_k)/(jw - conj(s_k)) with n_modes complex poles (initialised from peaks(),
    refined by scipy.optimize.least_squares on complex Z, relative weighting, f <= 1.1 f_max). Returns [(s_k, R_k)]
    sorted by Im(s_k), all Re(s_k) < 0 and Re(R_k) > 0."""
    f = np.asarray(f, dtype=float)
    z = np.asarray(z, dtype=complex)
    pk = [p for p in peaks(f, z) if p[0] <= f_max][:n_modes]
    if len(pk) < n_modes:
        raise ValueError(f"only {len(pk)} peaks below {f_max} Hz, need {n_modes}")
    sel = f <= 1.1 * f_max
    w, zs = 2 * np.pi * f[sel], z[sel]
    weight = 1.0 / np.abs(zs)
    x0, lo, hi = [], [], []
    for fp, amp, q in pk:
        sharp = np.isfinite(q) and q > 5.0
        q = q if np.isfinite(q) and q > 0.55 else 30.0
        wn = 2 * np.pi * fp
        sig = complex(-wn / (2 * q), wn * np.sqrt(1 - 1 / (4 * q * q)))
        r0 = amp * (wn / q) * sig / (sig - sig.conjugate())            # peak height of a single resonator
        span = 0.04 if sharp else 0.15
        x0 += [sig.real / _SC[0], sig.imag / _SC[1], r0.real / _SC[2], r0.imag / _SC[3]]
        lo += [-np.inf, sig.imag * (1 - span) / _SC[1], 0.0, -np.inf]
        hi += [-1e-6, sig.imag * (1 + span) / _SC[1], np.inf, np.inf]
    x0, lo, hi = np.array(x0), np.array(lo), np.array(hi)
    with np.errstate(invalid="ignore"):
        x0 = np.where(np.isfinite(lo), np.maximum(x0, lo + 1e-9 * (1 + abs(lo))), x0)
        x0 = np.where(np.isfinite(hi), np.minimum(x0, hi - 1e-9 * (1 + abs(hi))), x0)

    def res(x):
        zm, _ = _model(w, x)
        d = (zm - zs) * weight
        return np.concatenate([d.real, d.imag])

    fit = least_squares(res, x0, jac=lambda x: _jac(w, x, weight), bounds=(lo, hi), method="trf", x_scale="jac",
                        ftol=tol, xtol=tol, gtol=tol, max_nfev=max_nfev)
    p = fit.x.reshape(-1, 4) * _SC
    modes = sorted(((complex(a, b), complex(c, d)) for a, b, c, d in p), key=lambda m: m[0].imag)
    assert all(m[0].real < 0 and m[1].real > 0 for m in modes)
    return modes
