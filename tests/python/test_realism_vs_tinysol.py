# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-029 realism against the TinySOL "Trumpet in C" recordings (A-013, D-017, SC-4).

Needs env TPT_RENDER (tpt_render binary) and TINYSOL_DIR (download root; downloaded if missing). Skipped otherwise
(local runs); the CI realism job sets both. Thresholds were checked for feasibility before freezing
(lesson L-20261007T150400Z):
  harmonic MAD - neighbour model mean 1.5-2.6 dB, q90 <= 4.2 dB (C-073); model spikes pp 4.3-6.6, mf 2.3-4.6,
                 ff 2.0-4.2 dB (C-085, C-094)
  centroid     - neighbour coverage in [0.75, 1.333]: 100/100/93 % (C-073)
  onset        - TinySOL is trimmed at the attack (C-096): absolute bound only; spike medians 47-102 ms (C-089)
  ordering     - reference 100 % (C-074)
"""
import json, os, pathlib, subprocess, sys
import numpy as np
import pytest

ROOT = pathlib.Path(__file__).resolve().parents[2]
RENDER = os.environ.get("TPT_RENDER")
TINYSOL = os.environ.get("TINYSOL_DIR")
pytestmark = pytest.mark.skipif(not RENDER or not TINYSOL, reason="T-029 needs TPT_RENDER and TINYSOL_DIR (CI realism job)")

THRESH = {"mad_mean_db": 6.0, "mad_note_db": 10.0, "mad_note_fraction": 0.90,
          "centroid_band": (0.75, 1.333), "centroid_fraction": 0.85, "onset_median_ms": 150.0, "order_fraction": 0.90}


@pytest.fixture(scope="module")
def rows(tmp_path_factory):
    out = tmp_path_factory.mktemp("t029") / "results.json"
    subprocess.run([sys.executable, "-m", "tools.realism.compare_tinysol", "--tinysol", TINYSOL, "--render", RENDER,
                    "--out", str(out)], check=True, cwd=ROOT)
    return json.loads(out.read_text())


def test_t029_coverage(rows):
    assert len(rows) >= 80        # 86 TpC notes in concert 54..82 minus 4 resampled = 82


@pytest.mark.parametrize("dyn", ["pp", "mf", "ff"])
def test_t029_harmonics_centroid_onset(rows, dyn):
    rs = [r for r in rows if r["dyn"] == dyn]
    mad = np.array([r["harm_mad_db"] for r in rs])
    cr = np.array([r["centroid_ratio"] for r in rs])
    on = np.array([r["onset_syn_ms"] for r in rs])
    assert mad.mean() <= THRESH["mad_mean_db"]
    assert np.mean(mad <= THRESH["mad_note_db"]) >= THRESH["mad_note_fraction"]
    lo, hi = THRESH["centroid_band"]
    assert np.mean((cr >= lo) & (cr <= hi)) >= THRESH["centroid_fraction"]
    assert np.median(on) <= THRESH["onset_median_ms"]


def test_t029_dynamics_ordering(rows):
    by = {(r["concert"], r["dyn"]): r for r in rows}
    pitches = sorted({c for c, _ in by if all((c, d) in by for d in ("pp", "mf", "ff"))})
    ok = [c for c in pitches if by[(c, "pp")]["ncentroid_syn"] < by[(c, "mf")]["ncentroid_syn"] < by[(c, "ff")]["ncentroid_syn"]]
    assert len(ok) >= THRESH["order_fraction"] * len(pitches)


def test_t029_tinysol_selection():
    from tools.realism import tinysol
    notes = tinysol.trumpet_notes(tinysol.download(TINYSOL))
    assert len(notes) == 82
    import re
    assert all(not re.search(r"(^|_)R\d+[ud]$", pathlib.Path(n["path"]).stem.split("-")[-1]) for n in notes)
    assert all(54 <= n["concert"] <= 82 for n in notes)
