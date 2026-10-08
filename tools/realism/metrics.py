# SPDX-License-Identifier: Apache-2.0
"""Realism metrics shared by the frozen tests and tools (D-017). Definitions are those of
research/spikes/ref_spread.py; this module is the implementation (S-004). STUB (D-018)."""
import numpy as np

H_COMPARE = 8          # harmonics 1..8 for the harmonic MAD


def frames_db(x, fs, win):
    """Per-frame RMS level (dB re the loudest frame) for non-overlapping frames of `win` seconds."""
    raise NotImplementedError("frames_db")


def steady_segment(x, fs):
    """Longest run of 10 ms frames within 6 dB of the maximum, starting >= 0.15 s after the first frame above -40 dB,
    at most 1.0 s long."""
    raise NotImplementedError("steady_segment")


def harmonic_levels_db(x, fs, f0, count=10):
    """Levels (dB re the strongest) of harmonics 1..count: Hann window, zero padding to the next power of two >= 4 len,
    maximum magnitude bin within +-3 % of h f0."""
    raise NotImplementedError("harmonic_levels_db")


def spectral_centroid_hz(x, fs):
    """Power-spectrum centroid over [20 Hz, min(10 kHz, fs/2)], Hann window; 0.0 for silence."""
    raise NotImplementedError("spectral_centroid_hz")


def onset_ms(x, fs):
    """Synthetic-onset metric (D-017): 1 ms frames; from the first frame above -40 dB re the global maximum to the first
    frame >= the maximum of the following 300 ms minus 6.0206 dB. Meaningful only for signals that start in silence
    (the TinySOL files are trimmed at the attack, C-096)."""
    raise NotImplementedError("onset_ms")


def harmonic_mad_db(h_syn, h_ref, count=H_COMPARE):
    """Mean absolute difference (dB) of the first `count` harmonic levels."""
    raise NotImplementedError("harmonic_mad_db")
