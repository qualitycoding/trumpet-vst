# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-030 resonator tools (D-004; C-076, C-077, C-079, C-099): TMM checks, bore fit, deterministic generator, committed
table up to date, open state equal to the measured modes."""
import json, math, pathlib, subprocess, sys
import numpy as np
import pytest
from tools.resonator import tmm, bore_fit

ROOT = pathlib.Path(__file__).resolve().parents[2]
FIX = ROOT / "tests" / "fixtures"


def measured():
    rows = [l.split() for l in (FIX / "freour2022_open_modes.txt").read_text().splitlines() if l.strip() and not l.startswith("#")]
    return [(complex(float(a), float(b)), complex(float(c), float(d))) for a, b, c, d in rows]


def test_t030_cylinder_lossless():
    f = np.linspace(20, 3000, 29801)
    r, L = 7.5e-3, 0.5
    z, _ = tmm.input_impedance(f, [(0.0, r), (L, r)], 0.0, 0.0, r, lossless=True)
    pk = [p[0] for p in tmm.peaks(f, z)][:3]
    exp = [(2 * n - 1) * tmm.C27 / (4 * (L + 0.6133 * r)) for n in (1, 2, 3)]
    np.testing.assert_allclose(pk, exp, rtol=2e-3)


def test_t030_cone_vs_staircase():
    f = np.linspace(20, 2000, 19801)
    z1, _ = tmm.input_impedance(f, [(0.0, 5e-3), (1.0, 40e-3)], 0.0, 0.0, 5e-3)
    xs = np.linspace(0, 1.0, 2001)
    rs = 5e-3 + 35e-3 * xs
    prof = []
    for i in range(2000):
        rm = 0.5 * (rs[i] + rs[i + 1])
        prof += [(xs[i], rm), (xs[i + 1] - 1e-9, rm)]
    z2, _ = tmm.input_impedance(f, prof, 0.0, 0.0, 5e-3)
    p1 = [p[0] for p in tmm.peaks(f, z1)][:6]
    p2 = [p[0] for p in tmm.peaks(f, z2)][:6]
    np.testing.assert_allclose(p1, p2, rtol=2e-3)


def test_t030_bore_fit():
    res = bore_fit.fit()
    assert len(res["cents_err"]) == 10
    assert max(abs(c) for c in res["cents_err"]) <= 6.0


@pytest.fixture(scope="module")
def generated(tmp_path_factory):
    d = tmp_path_factory.mktemp("t030")
    outs = []
    for k in range(2):
        out = d / f"t{k}.json"
        subprocess.run([sys.executable, "-m", "tools.resonator.generate_table", "--fingerings", str(FIX / "trumpet_fingerings.txt"),
                        "--out", str(out), "--keep-calibration", str(ROOT / "data" / "trumpet_resonators.json")], check=True, cwd=ROOT)
        outs.append(out)
    return outs


def test_t030_deterministic(generated):
    a, b = (json.loads(p.read_text()) for p in generated)
    a.pop("provenance", None); b.pop("provenance", None)
    assert a == b


def test_t030_committed_table_is_current(generated):
    gen = json.loads(generated[0].read_text())
    com = json.loads((ROOT / "data" / "trumpet_resonators.json").read_text())
    assert com["format"] == "tpt-resonators-1"
    assert [s["id"] for s in gen["states"]] == [s["id"] for s in com["states"]]
    for sg, sc in zip(gen["states"], com["states"]):
        for mg, mc in zip(sg["modes"], sc["modes"]):
            np.testing.assert_allclose(mg["s"] + mg["R"], mc["s"] + mc["R"], rtol=1e-9)
    key = lambda n: (n["written"], n["kind"])
    assert sorted(map(key, gen["notes"])) == sorted(map(key, com["notes"]))
    gn = {key(n): n for n in gen["notes"]}
    for n in com["notes"]:
        assert n["natural_dev_cents"] == pytest.approx(gn[key(n)]["natural_dev_cents"], abs=1e-6)
        assert n["fscale"] == gn[key(n)]["fscale"]        # --keep-calibration copies it


def test_t030_open_state_is_measured():
    com = json.loads((ROOT / "data" / "trumpet_resonators.json").read_text())
    open_state = next(s for s in com["states"] if s["id"] == "0")
    for (s, c), m in zip(measured(), open_state["modes"]):
        assert complex(*m["s"]) == pytest.approx(s, rel=1e-9)
        assert complex(*m["R"]) == pytest.approx(c, rel=1e-9)
    assert len(open_state["modes"]) >= 13
