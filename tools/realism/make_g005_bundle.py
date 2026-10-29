# SPDX-License-Identifier: Apache-2.0
"""Builds the G-005 evidence bundle (plan/GATES.md): 12 blind pairs with a base64 key, the ten demos, SUMMARY.md and
results.json. Usage: python -m tools.realism.make_g005_bundle --tinysol <dir> --render <tpt_render> --out renders/G-005"""
import argparse
import base64
import json
import pathlib
import random
import subprocess
import sys
import tempfile

import numpy as np
import soundfile as sf

from . import compare_tinysol, metrics, summarize, tinysol

ROOT = pathlib.Path(__file__).resolve().parents[2]
FS = 44100
TARGET_RMS_DB = -23.0
FADE_S = 0.010
SEED = 20261008


def _mono(x):
    x = np.asarray(x, dtype=float)
    return x.mean(axis=1) if x.ndim > 1 else x


def _resample(x, fs_from, fs_to):
    if fs_from == fs_to:
        return x
    from scipy.signal import resample_poly
    from math import gcd
    g = gcd(int(fs_from), int(fs_to))
    return resample_poly(x, int(fs_to) // g, int(fs_from) // g)


def clip(x, fs):
    """1.0 s steady segment (metrics.steady_segment), RMS -23 dBFS, 10 ms fades, at 44.1 kHz."""
    seg = _resample(np.asarray(metrics.steady_segment(x, fs), dtype=float), fs, FS)
    seg = seg - np.mean(seg)
    rms = np.sqrt(np.mean(seg ** 2))
    seg = seg * (10 ** (TARGET_RMS_DB / 20) / max(rms, 1e-12))
    n = int(FADE_S * FS)
    ramp = 0.5 - 0.5 * np.cos(np.pi * np.arange(n) / n)
    seg[:n] *= ramp
    seg[-n:] *= ramp[::-1]
    return seg


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--tinysol", required=True)
    ap.add_argument("--render", required=True)
    ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    out = pathlib.Path(a.out)
    (out / "blind").mkdir(parents=True, exist_ok=True)
    (out / "demos").mkdir(parents=True, exist_ok=True)
    root = tinysol.download(a.tinysol)
    notes = tinysol.trumpet_notes(root)

    rng = random.Random(SEED)
    pitches = sorted({n["concert"] for n in notes if n["dyn"] == "mf"})
    pick = rng.sample(pitches, 12)
    key = {}
    with tempfile.TemporaryDirectory() as tmp:
        for i, concert in enumerate(pick, 1):
            real_is_a = rng.random() < 0.5
            ref_note = next(n for n in notes if n["concert"] == concert and n["dyn"] == "mf")
            wav = pathlib.Path(tmp) / f"syn-{concert}.wav"
            subprocess.run([a.render, "--note", str(concert), "--velocity", f"{76 / 127:.9f}", "--seconds", "2.0", "--fs", str(FS),
                            "--out", str(wav)], check=True)
            syn, fs_s = sf.read(wav)
            ref, fs_r = sf.read(ref_note["path"])
            real, fake = clip(_mono(ref), fs_r), clip(_mono(syn), fs_s)
            name = f"pair-{i:02d}"
            a_clip, b_clip = (real, fake) if real_is_a else (fake, real)
            sf.write(out / "blind" / f"{name}-A.wav", a_clip, FS, subtype="PCM_24")
            sf.write(out / "blind" / f"{name}-B.wav", b_clip, FS, subtype="PCM_24")
            key[name] = "A" if real_is_a else "B"
    (out / "blind" / "key.json.b64").write_text(base64.b64encode(json.dumps(key, sort_keys=True).encode()).decode() + "\n")

    for f in sorted((ROOT / "tools" / "realism" / "demos").glob("*.json")):
        subprocess.run([a.render, "--events", str(f), "--fs", "48000", "--out", str(out / "demos" / (f.stem + ".wav"))], check=True)

    results = out / "results.json"
    compare_tinysol.main(["--tinysol", a.tinysol, "--render", a.render, "--out", str(results)])
    summarize.main([str(results), "--out", str(out / "SUMMARY.md")])
    print(f"bundle written to {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
