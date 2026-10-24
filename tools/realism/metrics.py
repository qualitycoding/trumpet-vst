# SPDX-License-Identifier: Apache-2.0
"""Realism metrics shared by the frozen tests and tools (D-017). Definitions are those of
research/spikes/ref_spread.py; this module is the implementation (S-004)."""
import numpy as np

H_COMPARE = 8          # harmonics 1..8 for the harmonic MAD


def frames_db(x, fs, win):
    """Per-frame RMS level (dB re the loudest frame) for non-overlapping frames of `win` seconds."""
    n = int(win * fs)
    m = len(x) // n
    e = np.sqrt(np.mean(np.asarray(x[: m * n], dtype=float).reshape(m, n) ** 2, axis=1)) + 1e-12
    return 20 * np.log10(e / e.max()), n


def steady_segment(x, fs):
    """Longest run of 10 ms frames within 6 dB of the maximum, starting >= 0.15 s after the first frame above -40 dB,
    at most 1.0 s long."""
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


def _spectrum(x):
    x = np.asarray(x, dtype=float)
    n_fft = 1 << int(np.ceil(np.log2(4 * len(x))))
    return np.abs(np.fft.rfft(x * np.hanning(len(x)), n_fft)), n_fft


def harmonic_levels_db(x, fs, f0, count=10):
    """Levels (dB re the strongest) of harmonics 1..count: Hann window, zero padding to the next power of two >= 4 len,
    maximum magnitude bin within +-3 % of h f0."""
    mag, n_fft = _spectrum(x)
    fr = np.fft.rfftfreq(n_fft, 1 / fs)
    lv = []
    for h in range(1, count + 1):
        lo, hi = np.searchsorted(fr, [h * f0 * 0.97, h * f0 * 1.03])
        lv.append(mag[lo:hi].max() if hi > lo else 1e-12)
    lv = 20 * np.log10(np.array(lv) + 1e-12)
    return lv - lv.max()


def spectral_centroid_hz(x, fs):
    """Power-spectrum centroid over [20 Hz, min(10 kHz, fs/2)], Hann window; 0.0 for silence."""
    mag, n_fft = _spectrum(x)
    fr = np.fft.rfftfreq(n_fft, 1 / fs)
    p = mag ** 2
    sel = (fr >= 20) & (fr <= min(10000.0, fs / 2))
    total = float(np.sum(p[sel]))
    return float(np.sum(fr[sel] * p[sel]) / total) if total > 0 else 0.0


def onset_ms(x, fs):
    """Synthetic-onset metric (D-017): 1 ms frames; from the first frame above -40 dB re the global maximum to the first
    frame >= the maximum of the following 300 ms minus 6.0206 dB. Meaningful only for signals that start in silence
    (the TinySOL files are trimmed at the attack, C-096)."""
    db, _ = frames_db(x, fs, 0.001)
    i0 = int(np.argmax(db > -40))
    seg = db[i0: i0 + 300]
    return float(int(np.argmax(seg >= seg.max() - 6.0206)))


def harmonic_mad_db(h_syn, h_ref, count=H_COMPARE):
    """Mean absolute difference (dB) of the first `count` harmonic levels."""
    return float(np.mean(np.abs(np.asarray(h_syn[:count], dtype=float) - np.asarray(h_ref[:count], dtype=float))))
