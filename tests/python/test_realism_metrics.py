# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-028 realism metric definitions (D-017) on synthetic signals."""
import numpy as np
import pytest
from tools.realism import metrics as m

FS = 44100


def tone(f, amps, sec=1.5, fs=FS):
    t = np.arange(int(sec * fs)) / fs
    return sum(a * np.sin(2 * np.pi * (k + 1) * f * t) for k, a in enumerate(amps))


def test_t028_harmonic_levels():
    x = tone(220.0, [1.0, 0.5, 0.25, 0.125])
    h = m.harmonic_levels_db(x, FS, 220.0, 4)
    assert len(h) == 4
    assert h[0] == pytest.approx(0.0, abs=0.3)
    assert h[1] == pytest.approx(-6.020599913279624, abs=0.3)    # EXPECTED:db_half
    assert h[2] == pytest.approx(-12.041199826559248, abs=0.3)   # EXPECTED:db_quarter


def test_t028_centroid_and_mad():
    x = tone(1000.0, [1.0])
    assert m.spectral_centroid_hz(x, FS) == pytest.approx(1000.0, rel=0.01)
    assert m.spectral_centroid_hz(np.zeros(1000), FS) == 0.0
    assert m.harmonic_mad_db([0, -3, -6, -9, -12, -15, -18, -21, -99], [0, -4, -6, -8, -12, -16, -18, -20, 0]) == pytest.approx(0.5)


def test_t028_onset_metric():
    # linear 100 ms ramp from silence: -40 dB re max at 1 ms, half amplitude (-6.02 dB) at 50 ms -> about 49 ms
    t = np.arange(int(1.0 * FS)) / FS
    x = np.concatenate([np.zeros(int(0.05 * FS)), np.clip(t / 0.1, 0, 1) * np.sin(2 * np.pi * 440 * t)])
    assert m.onset_ms(x, FS) == pytest.approx(49.0, abs=3.0)
    slow = np.concatenate([np.zeros(int(0.05 * FS)), np.clip(t / 0.2, 0, 1) * np.sin(2 * np.pi * 440 * t)])
    assert m.onset_ms(slow, FS) > m.onset_ms(x, FS) + 30.0     # a slower attack measures later (L-20261008T010500Z)


def test_t028_steady_segment():
    t = np.arange(int(2.0 * FS)) / FS
    env = np.where(t < 1.6, np.clip(t / 0.05, 0, 1), np.clip(1 - (t - 1.6) / 0.2, 0, 1))
    seg = m.steady_segment(env * np.sin(2 * np.pi * 300 * t), FS)
    assert 0.9 <= len(seg) / FS <= 1.0
