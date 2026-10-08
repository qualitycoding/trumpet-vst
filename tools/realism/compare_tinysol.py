# SPDX-License-Identifier: Apache-2.0
"""Render every reference note with tpt_render and compute the realism metrics (D-017). STUB (D-018), S-015.

Usage: python -m tools.realism.compare_tinysol --tinysol <root> --render <path to tpt_render> --out results.json
Writes a JSON list of rows {concert, dyn, velocity, harm_syn, harm_ref, harm_mad_db, centroid_syn, centroid_ref,
centroid_ratio, ncentroid_syn, ncentroid_ref, onset_syn_ms} and a sidecar results.meta.json {n_rows, excluded, git}.
Velocities: pp 24/127, mf 76/127, ff 124/127; renders 2.0 s at 44100 Hz, default parameters. f0 = 12-TET Hz of the concert
pitch (A4 = 440) on both sides; harmonics (count=10, MAD over the first 8) and centroids on metrics.steady_segment of each
signal; onset_syn_ms on the full render. --tinysol is the download destination (download() is called here)."""
VELOCITY = {"pp": 24 / 127, "mf": 76 / 127, "ff": 124 / 127}


def main(argv=None):
    raise NotImplementedError("compare_tinysol.main")


if __name__ == "__main__":
    raise SystemExit(main())
