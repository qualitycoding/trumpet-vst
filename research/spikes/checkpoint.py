"""Write .checkpoints/state.json (protocol 1.2) with SHA-256 hashes of every tracked planning artifact.

Usage: python3 research/spikes/checkpoint.py <phase> <subphase> --completed a,b --pending c,d [--round k=v ...]
Existing fields (lessons, knowledge, substitutions, open items) are preserved and merged.
"""
import argparse, datetime, hashlib, json, pathlib, subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]
STATE = ROOT / ".checkpoints" / "state.json"
SKIP = {".checkpoints/state.json"}


def artifacts():
    files = subprocess.run(["git", "ls-files", "--cached", "--others", "--exclude-standard"],
                           cwd=ROOT, capture_output=True, text=True, check=True).stdout.split()
    out = {}
    for f in sorted(files):
        if f in SKIP or f.startswith(("LICENSE", "NOTICE", "README")):
            continue
        p = ROOT / f
        if p.is_file():
            out[f] = hashlib.sha256(p.read_bytes()).hexdigest()
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("phase"); ap.add_argument("subphase")
    ap.add_argument("--completed", default=""); ap.add_argument("--pending", default="")
    ap.add_argument("--round", action="append", default=[])
    ap.add_argument("--open", action="append", default=[])
    a = ap.parse_args()
    s = json.loads(STATE.read_text()) if STATE.exists() else {}
    s.setdefault("run_id", "20261007T145241Z-trumpet-vst-plan")
    s.setdefault("branch", "gen-20261007T145241Z-trumpet-vst-plan")
    s.setdefault("profiles", ["software"])
    s.setdefault("mode_flags", {"software.deploys": False})
    for k in ("research_round", "exploration_round", "proof_review_round", "premortem_round"):
        s.setdefault(k, 0)
    s["phase"], s["subphase"] = a.phase, a.subphase
    done = [x for x in a.completed.split(",") if x]
    s["completed"] = sorted(set(s.get("completed", [])) | set(done))
    s["pending"] = [x for x in a.pending.split(",") if x]
    s.setdefault("not_applicable", ["0.3.3", "0.3.4", "0.3.5", "R2b", "2A", "2B.2", "2B.4", "2C", "2D",
                                    "plan/OPERATIONS.md", "G-001"])
    for kv in a.round:
        k, v = kv.split("="); s[k] = int(v)
    s["open_items"] = sorted(set(s.get("open_items", [])) | set(a.open))
    s.setdefault("tier_substitutions", [])
    s.setdefault("knowledge_store", {"location": "qualitycoding/agent-knowledge", "commit_read": None,
                                     "readable": False, "note": "does not exist (HTTP 404, 2026-10-07)"})
    s.setdefault("lessons_loaded", [])
    s["lessons_recorded"] = sorted(p.stem for p in (ROOT / "lessons").glob("L-*.md"))
    s["knowledge_added"] = sorted(p.stem for p in (ROOT / "knowledge").glob("K-*.md"))
    s["artifacts"] = artifacts()
    s["updated_at"] = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    STATE.parent.mkdir(exist_ok=True)
    STATE.write_text(json.dumps(s, indent=1, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()
