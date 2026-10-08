"""Computes the T-028 expected onset values with the reference metric (research/spikes/ref_spread.onset_ms) on the
exact T-028 signals (lesson L-20261007T150300Z; cold-read pass 1 item 23). Values are copied into expected_values.py
(onset_ramp100_ms, onset_ramp200_ms); this script asserts they still match. Needs numpy."""
import numpy as np
import ref_spread as ref

FS = 44100


def signals():
    t = np.arange(int(1.0 * FS)) / FS
    fast = np.concatenate([np.zeros(int(0.05 * FS)), np.clip(t / 0.1, 0, 1) * np.sin(2 * np.pi * 440 * t)])
    slow = np.concatenate([np.zeros(int(0.05 * FS)), np.clip(t / 0.2, 0, 1) * np.sin(2 * np.pi * 440 * t)])
    return fast, slow


if __name__ == "__main__":
    import expected_values as ev
    fast, slow = signals()
    got = (float(ref.onset_ms(fast, FS)), float(ref.onset_ms(slow, FS)))
    v = ev.values()
    assert got == (v["onset_ramp100_ms"], v["onset_ramp200_ms"]), got
    print("onset_ramp100_ms", repr(float(ref.onset_ms(fast, FS))), "onset_ramp200_ms", repr(float(ref.onset_ms(slow, FS))))
