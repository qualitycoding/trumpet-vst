# SPDX-License-Identifier: Apache-2.0
"""Plane-wave transfer-matrix model of the trumpet bore (D-004). Reference implementation:
research/spikes/tmm_trumpet.py (cone matrix verified by the R5 review). STUB (D-018), implemented in S-005."""
C27 = 331.3 * (1 + 27.0 / 273.15) ** 0.5     # m/s at 27 degC
RHO27 = 1.2041 * 293.15 / (273.15 + 27.0)    # kg/m^3 at 27 degC


def cone_matrix(f, r1, r2, length, lossless=False, c=C27, rho=RHO27):
    """2x2xN transfer matrix (p, U) of a conical frustum (cylinder when |r2 - r1| < 1e-7; identity when length < 1e-8 m,
    where the spherical-wave form is singular)."""
    raise NotImplementedError("cone_matrix")


def input_impedance(f, profile, valve_x, extra_length, valve_r, lossless=False, c=C27, rho=RHO27):
    """(Zin, H) of the bore: profile = [(x, r), ...]; a cylinder of extra_length and radius valve_r inserted at valve_x;
    unflanged radiation at the last point; H = p_bell / p_in."""
    raise NotImplementedError("input_impedance")


def peaks(f, z):
    """List of (f_peak, |z|_peak, Q) for every local maximum of |z| (half-power Q, parabolic refinement)."""
    raise NotImplementedError("peaks")


def complex_modal_fit(f, z, n_modes, f_max):
    """Fit Z(w) = sum_k R_k/(jw - s_k) + conj(R_k)/(jw - conj(s_k)) with n_modes complex poles (initialised from peaks(),
    refined by scipy.optimize.least_squares on complex Z, relative weighting, f <= 1.1 f_max). Returns [(s_k, R_k)]
    sorted by Im(s_k), all Re(s_k) < 0 and Re(R_k) > 0."""
    raise NotImplementedError("complex_modal_fit")
