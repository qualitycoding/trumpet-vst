# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-031 supply chain and licences (D-020, D-021, A-016; C-001..C-022, C-097, C-098; SC-10)."""
import pathlib, re

ROOT = pathlib.Path(__file__).resolve().parents[2]


def test_t031_fetchcontent_pins():
    tags = re.findall(r"GIT_TAG\s+(\S+)", (ROOT / "CMakeLists.txt").read_text())
    assert len(tags) >= 3
    assert all(re.fullmatch(r"[0-9a-f]{40}", t) for t in tags)
    assert "be29c81492b6151c8ea8d14c840e1311963b3a83" in tags     # JUCE 9.0.3


def test_t031_python_lock():
    lock = [l for l in (ROOT / "tools" / "requirements.lock").read_text().splitlines() if l.strip() and not l.startswith("#")]
    assert lock and all(re.fullmatch(r"[A-Za-z0-9_.\-]+==[0-9][\w.\-]*", l) for l in lock)
    req = (ROOT / "tools" / "requirements.txt").read_text()
    assert all(re.search(rf"^{p}==", req, re.M) for p in ("numpy", "scipy", "soundfile", "pytest", "pip-audit"))


def test_t031_workflows_pinned_and_least_privilege():
    wfs = list((ROOT / ".github" / "workflows").glob("*.yml"))
    assert wfs, "no CI workflow"
    for wf in wfs:
        text = wf.read_text()
        uses = re.findall(r"uses:\s*([^\s#]+)", text)
        assert all(re.fullmatch(r"[\w.\-/]+@[0-9a-f]{40}", u) for u in uses), wf
        assert "pull_request_target" not in text and "workflow_run" not in text
        assert re.search(r"^permissions:\s*\n\s+contents:\s*read", text, re.M), wf
        assert "pip-install:" not in text      # removed in setup-python v7 (C-097)


def test_t031_notices_and_attribution():
    tpn = (ROOT / "THIRD_PARTY_NOTICES.md").read_text()
    for name in ("JUCE", "VST3 SDK", "MIT", "AGPL", "Catch2", "nlohmann", "pluginval", "TinySOL", "Fréour"):
        assert name in tpn, name
    att = (ROOT / "ATTRIBUTION.md").read_text()
    assert "10.5281/zenodo.3685367" in att and "10.1051/aacus/2022004" in att and "CC BY 4.0" in att
    assert "AGPLv3" in (ROOT / "NOTICE").read_text()
    assert not list(ROOT.glob("**/*.wav")) or all("reference-data" in str(p) or "renders" in str(p) or "build" in str(p) for p in ROOT.glob("**/*.wav"))
