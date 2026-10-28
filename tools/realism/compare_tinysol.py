# SPDX-License-Identifier: Apache-2.0
"""Render every reference note with tpt_render and compute the realism metrics (D-017).

Usage: python -m tools.realism.compare_tinysol --tinysol <root> --render <path to tpt_render> --out results.json
Writes a JSON list of rows {concert, dyn, velocity, harm_syn, harm_ref, harm_mad_db, centroid_syn, centroid_ref,
centroid_ratio, ncentroid_syn, ncentroid_ref, onset_syn_ms} and a sidecar results.meta.json {n_rows, excluded, git}.
Velocities: pp 24/127, mf 76/127, ff 124/127; renders 2.0 s at 44100 Hz, default parameters. f0 = 12-TET Hz of the concert
pitch (A4 = 440) on both sides; harmonics (count=10, MAD over the first 8) and centroids on metrics.steady_segment of each
signal; onset_syn_ms on the full render. --tinysol is the download destination (download() is called here)."""
import argparse
import json
import pathlib
import subprocess
import sys
import tempfile

import numpy as np
import soundfile as sf

from . import metrics, tinysol

VELOCITY = {"pp": 24 / 127, "mf": 76 / 127, "ff": 124 / 127}
FS_RENDER = 44100
SECONDS = 2.0
ROOT = pathlib.Path(__file__).resolve().parents[2]


def _mono(x):
    x = np.asarray(x, dtype=float)
    return x.mean(axis=1) if x.ndim > 1 else x


def _git():
    try:
        return subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def _row(note, render_exe, tmp):
    f0 = 440.0 * 2 ** ((note["concert"] - 69) / 12)
    vel = VELOCITY[note["dyn"]]
    wav = pathlib.Path(tmp) / f"{note['concert']}-{note['dyn']}.wav"
    subprocess.run([str(render_exe), "--note", str(note["concert"]), "--velocity", f"{vel:.9f}", "--seconds", str(SECONDS),
                    "--fs", str(FS_RENDER), "--out", str(wav)], check=True)
    syn, fs_syn = sf.read(wav)
    ref, fs_ref = sf.read(note["path"])
    syn, ref = _mono(syn), _mono(ref)
    seg_s, seg_r = metrics.steady_segment(syn, fs_syn), metrics.steady_segment(ref, fs_ref)
    h_s = metrics.harmonic_levels_db(seg_s, fs_syn, f0, 10)
    h_r = metrics.harmonic_levels_db(seg_r, fs_ref, f0, 10)
    c_s, c_r = metrics.spectral_centroid_hz(seg_s, fs_syn), metrics.spectral_centroid_hz(seg_r, fs_ref)
    return {"concert": note["concert"], "dyn": note["dyn"], "velocity": vel,
            "harm_syn": [float(v) for v in h_s], "harm_ref": [float(v) for v in h_r],
            "harm_mad_db": metrics.harmonic_mad_db(h_s, h_r),
            "centroid_syn": c_s, "centroid_ref": c_r, "centroid_ratio": c_s / c_r if c_r > 0 else float("nan"),
            "ncentroid_syn": c_s / f0, "ncentroid_ref": c_r / f0, "onset_syn_ms": metrics.onset_ms(syn, fs_syn)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--tinysol", required=True)
    ap.add_argument("--render", required=True)
    ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    root = tinysol.download(a.tinysol)
    notes = tinysol.trumpet_notes(root)
    with tempfile.TemporaryDirectory() as tmp:
        rows = [_row(n, a.render, tmp) for n in notes]
    out = pathlib.Path(a.out)
    out.write_text(json.dumps(rows))
    out.with_name("results.meta.json").write_text(json.dumps({"n_rows": len(rows), "excluded": "4 semitone-resampled files (C-096)",
                                                              "git": _git()}))
    print(f"{len(rows)} rows -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
