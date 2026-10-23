"""Builds research/PRIOR_KNOWLEDGE.md from the R0 triage results in research/r0/ (store lessons and knowledge items,
re-run on 2026-10-22 after the human identified the store) and from this run's own lessons in lessons/.

Usage (repository root): python3 research/spikes/build_prior_knowledge.py
"""
import json, pathlib, re

R = pathlib.Path(__file__).resolve().parents[2]
SEV = {"Critical": 0, "High": 1, "Medium": 2, "Low": 3}


def own_lessons():
    out = []
    for p in sorted((R / "lessons").glob("L-*.md")):
        t = p.read_text(encoding="utf-8")
        f = dict(re.findall(r"^(\w+):[ \t]*(.*)$", t.split("---", 2)[1], re.M))
        rule = re.search(r"## Prevention rule\n(.*?)\n## ", t, re.S)
        out.append({"id": p.stem, "severity": f.get("severity", "").strip(), "recorded_at": f.get("recorded_at", ""),
                    "rule": " ".join(rule.group(1).split()) if rule else ""})
    return out


OVERRIDES = {
    "L-20261004T150300Z-fixture-cents-reference-undeclared": "partly applied: D-004 Cents convention (informative `provenance.cents_reference`; the parser stays lenient because the frozen T-006 fixture lacks it) and the S-005 Done-when check that the generated table carries it",
    "L-20261004T150400Z-interface-gap-found-only-by-implementing": "partly applied: PLAN.md Conventions 'Interfaces' (additive members, only a signature change halts); no planner-side walk of every declared function was done",
    "L-20261009T193617Z-manual-upload-bundle-hygiene": "applied: DECISIONS.md 'DR-CRED procedure' step 3 (UPLOAD.md, tree comparison)",
    "L-20261009T193604Z-zip-exclude-dotfiles-dropped-checkpoints": "applied: DECISIONS.md 'DR-CRED procedure' step 3 (zip from name-status, dot-directories included)",
    "L-20261009T120012Z-residual-noise-level-41-db-low-found-only-by-spectrogram": "amended: PLAN.md S-008 action 5 (`extras/core/test_noise_power.cpp`, `core/src/BreathNoise.h`) and D-012 Breath noise",
    "L-20261009T193612Z-frozen-test-silent-fallback-single-seed": "amended: PLAN.md S-001 action 3b (`extras/core/test_support_selftest.cpp`)",
    "L-20261009T161200Z-edocontrol-differs-from-plan-3-3-and-nothing-logged": "amended: PLAN.md S-014 action 5, Outputs and Done-when (`logs/S-014/ui-checklist.md`)",
    "L-20261009T140011Z-test-category-counts-in-the-traceability-report-were-estimat": "amended: PLAN.md S-018 action 3b (`tools/report/make_report.py`, generated blocks)",
    "L-20261009T140009Z-the-red-verification-classifier-mis-read-junit-failure-and-e": "amended: PLAN.md S-001 action 3c (`logs/S-001/red-summary.md`)",
}


def main():
    store = []
    for b in (1, 2, 3):
        store += json.load(open(R / f"research/r0/result_batch{b}.json"))
    meta = {x["id"]: x for x in json.load(open(R / "research/r0/lessons_all.json"))}
    for x in store:
        x["recorded_at"] = meta.get(x["id"], {}).get("recorded_at", "")
    store.sort(key=lambda x: (SEV.get(x["severity"], 9), "".join(chr(255 - ord(c)) for c in x["recorded_at"])))
    know = json.load(open(R / "research/r0/result_knowledge.json"))
    own = sorted(own_lessons(), key=lambda x: (SEV.get(x["severity"], 9), "".join(chr(255 - ord(c)) for c in x["recorded_at"])))
    lines = ["# Prior knowledge (R0) and how it is applied", "",
             "## Sources",
             "- **Store:** `qualitycoding/Lessons` (lessons, commit ac028c7) and `qualitycoding/knowledge` (knowledge items,",
             "  commit 09f3458), identified by the human on 2026-10-22 (A-018). R0 was re-run then, with the tag set of A-025.",
             "- **Selection:** 179 active lessons (`domain:`/`tech:` tag match, or `applies:all`) and 134 current knowledge items",
             "  (`research/r0/lessons_all.json`, `knowledge_all.json`).",
             "- **Triage:** done by three Sonnet agents for the lessons and one for the knowledge items. Results are in",
             "  `research/r0/result_*.json`. Every gap was turned into a plan amendment (commits 9a713cb and later).",
             "- **Reading the application table:** for gaps it quotes the triage *proposal* (truncated); the adopted text is in the",
             "  plan files and may differ (for example paths under `extras/`). Where a row and the plan differ, the plan wins.",
             "- **This run's own lessons:** " + str(len(own)) + " lessons in `lessons/`. Eight were backfilled from sibling runs (A-019); the",
             "  planner recorded the rest during this run.",
             "- **Aliases:** \"R5\" and \"R5 review\" refer to `research/rounds/round-3-adversarial.md`.", "",
             "## Prevention-rule checklist",
             "Used by the cold reads, the pre-mortem and the implementer. Order: severity, then most recent. Store lessons whose",
             "trigger cannot arise here are listed in the application table, not in this checklist.", "",
             "| # | Lesson | Severity | Prevention rule |", "|---|---|---|---|"]
    n = 0
    for x in own:
        n += 1
        lines.append(f"| {n} | {x['id']} (this run) | {x['severity']} | {x['rule'].replace('|', '/')} |")
    for x in store:
        if x["verdict"] == "not-applicable":
            continue
        n += 1
        lines.append(f"| {n} | {x['id']} | {x['severity']} | {x['prevention_rule'].replace('|', '/')} |")
    lines += ["", "## Application of every store lesson", "",
              "| Lesson | Severity | Triage verdict | Applied as / proposal / reason |", "|---|---|---|---|"]
    for x in store:
        if x["id"] in OVERRIDES:
            how = OVERRIDES[x["id"]]
        elif x["verdict"] == "applied":
            how = "applied: " + ", ".join(x.get("plan_refs", []))
        elif x["verdict"] == "gap":
            how = f"gap ({x.get('gap_severity')}) → amended (R0 re-run 2026-10-22): " + " ".join(x.get("amendment", "").split())[:220]
        else:
            how = "not applicable: " + " ".join(x.get("rationale", "").split())[:200]
        lines.append(f"| {x['id']} | {x['severity']} | {x['verdict']} | {how.replace('|', '/')} |")
    lines += ["", "## This run's own lessons: application", "",
              "Each one's application is described in the decisions, tests and steps that cite it (`grep -rn <id> plan tests`).", ""]
    for x in own:
        lines.append(f"- {x['id']} ({x['severity']})")
    lines += ["", "## Knowledge items used", "",
              "No store item is used as a *source* for a pin: their `applies_to_version` differs from the pinned versions, or they",
              "carry no version (protocol R0 step 4). The relevant ones are applied as follows (107 others are about unrelated",
              "projects).", "", "| Item | Verdict | Use |", "|---|---|---|"]
    for k in know:
        if k["verdict"] == "irrelevant":
            continue
        use = ", ".join(k.get("plan_refs", [])) if k["verdict"] == "relevant-consistent" else " ".join(k.get("amendment", "").split())[:200]
        lines.append(f"| {k['id']} | {k['verdict']} | {use.replace('|', '/')} |")
    (R / "research/PRIOR_KNOWLEDGE.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    import collections
    print("checklist rows", n, collections.Counter(x["verdict"] for x in store), len(own), "own")


if __name__ == "__main__":
    main()
