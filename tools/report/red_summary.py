#!/usr/bin/env python3
"""S-001 3c: classify the baseline red run. Exits non-zero if any test is unclassified."""
import re, subprocess, sys, xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SUITES = ["unit", "integration", "operational", "alloc", "perf"]
rows, bad = [], 0

for s in SUITES:
    out = subprocess.run([str(ROOT / f"build/tests/tpt_{s}_tests")], capture_output=True, text=True).stdout
    m = re.search(r"test cases:\s+(\d+)\s*\|(?:\s*(\d+) passed \|)?\s*(\d+) failed", out)
    if not m:
        rows.append((f"cpp:{s}", "?", "UNCLASSIFIED")); bad += 1; continue
    total, passed, failed = int(m[1]), int(m[2] or 0), int(m[3])
    reason = "NotImplemented (stub)" if "not implemented" in out or "NotImplemented" in out else "missing output"
    if passed == 0 and failed == total:
        rows.append((f"cpp:{s}", f"{failed}/{total} fail", reason))
    else:
        rows.append((f"cpp:{s}", f"{passed} pass, {failed} fail", "UNCLASSIFIED")); bad += 1

junit = ROOT / "logs/S-001/pytest.xml"
subprocess.run([str(ROOT / ".venv/bin/python"), "-m", "pytest", "tests/python", f"--junitxml={junit}", "-q"],
               cwd=ROOT, capture_output=True)
for tc in ET.parse(junit).getroot().iter("testcase"):
    name = f'{tc.get("classname","").split(".")[-1]}::{tc.get("name")}'
    kids = {c.tag: c for c in tc}
    if "skipped" in kids:
        rows.append((name, "skipped", "T-029 needs TINYSOL_DIR/TPT_RENDER")); continue
    bodies = " ".join((c.get("message") or "") + (c.text or "") for c in kids.values())
    if not kids:
        rows.append((name, "pass", "guard pass")); continue
    if "NotImplementedError" in bodies:
        reason = "NotImplemented"
    elif "FileNotFoundError" in bodies or "No such file" in bodies or "CalledProcessError" in bodies:
        reason = "missing output (S-002/S-005)"
    elif "AssertionError" in bodies or "assert" in bodies:
        reason = "missing output (S-002 notices)"
    else:
        rows.append((name, "fail", "UNCLASSIFIED")); bad += 1; continue
    rows.append((name, "fail" if "failure" in kids else "error", reason))

out = ["# S-001 red summary", "", "| Test | Status | Reason |", "|---|---|---|"]
out += [f"| {a} | {b} | {c} |" for a, b, c in rows]
(ROOT / "logs/S-001/red-summary.md").write_text("\n".join(out) + "\n")
print(f"{len(rows)} entries, {bad} unclassified")
sys.exit(1 if bad else 0)
