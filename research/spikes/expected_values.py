"""Single source of every numeric expected value used in frozen tests (lesson L-20261007T150300Z) and of the
arithmetic facts behind claims C-054, C-056, C-069.

Usage:
  python3 research/spikes/expected_values.py            # print all values (JSON)
  python3 research/spikes/expected_values.py --check    # verify every 'EXPECTED:<name>' literal in tests/
Each frozen test that uses one of these numbers carries a comment `// EXPECTED:<name>` (or `# EXPECTED:<name>`) on
the same line, followed by the literal; --check parses the literal and compares it with the value below
(relative difference < 1e-12).
"""
import json, math, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]


def et(midi, a4=440.0):
    return a4 * 2 ** ((midi - 69) / 12)


def values():
    v = {
        "et_69_440": et(69), "et_58_440": et(58), "et_52_440": et(52), "et_87_440": et(87), "et_82_442": et(82, 442.0),
        "et_46_440": et(46), "et_116_5": et(46),
        "cents_880_440": 1200 * math.log2(880 / 440), "cents_466_440": 1200 * math.log2(466 / 440),
    }
    # harmonic-series deviation from the nearest equal-tempered pitch (cents), partials 1..13 (C-054)
    for n in range(1, 14):
        st = 12 * math.log2(n)
        v[f"hs_dev_p{n}"] = 100 * (st - round(st))
    # partial-pitch rule (C-056): written pitch of partial n of the open horn = 48 + 12 log2 n (rounded)
    for n in range(1, 14):
        v[f"open_partial_written_p{n}"] = 48 + round(12 * math.log2(n))
    # ideal valve combination sharpness when each valve is cut for the open horn (C-069)
    r = {"1": 2 ** (2 / 12) - 1, "2": 2 ** (1 / 12) - 1, "3": 2 ** (3 / 12) - 1}
    for combo, st in (("12", 3), ("23", 4), ("13", 5), ("123", 6)):
        v[f"ideal_sharp_{combo}"] = 1200 * math.log2(2 ** (st / 12) / (1 + sum(r[c] for c in combo)))
    # analysis test signals (T-010)
    v["db_half"] = 20 * math.log10(0.5)
    v["db_quarter"] = 20 * math.log10(0.25)
    v["sine_rms_db"] = 20 * math.log10(1 / math.sqrt(2))
    v["cents_442_440"] = 1200 * math.log2(442 / 440)
    # Freour et al. 2022 Table 1 pole 4 frequency (Hz) and Fréour f_l
    v["freour_f4_hz"] = 2.9066e3 / (2 * math.pi)
    v["freour_fl_hz"] = 382.18
    return v


def check():
    v = values()
    bad = 0
    pat = re.compile(r"EXPECTED:(\w+)\s*\n?\s*.*?([-+]?\d+\.\d+(?:[eE][-+]?\d+)?)")
    for p in sorted(ROOT.glob("tests/**/*")):
        if not p.is_file() or p.suffix not in (".cpp", ".h", ".py"):
            continue
        for line in p.read_text(encoding="utf-8").splitlines():
            m = re.search(r"EXPECTED:(\w+)", line)
            if not m:
                continue
            name = m.group(1)
            nums = re.findall(r"[-+]?\d+\.\d+(?:[eE][-+]?\d+)?", line.split("EXPECTED:")[0])
            if name not in v or not nums:
                print(f"{p}: unknown or missing literal for {name}"); bad += 1; continue
            got = float(nums[-1])
            if abs(got - v[name]) > 1e-12 * max(1.0, abs(v[name])):
                print(f"{p}: {name} literal {got!r} != {v[name]!r}"); bad += 1
    print("expected values OK" if not bad else f"{bad} mismatches")
    return bad


if __name__ == "__main__":
    if "--check" in sys.argv:
        sys.exit(1 if check() else 0)
    print(json.dumps({k: repr(x) if isinstance(x, float) else x for k, x in values().items()}, indent=1))
