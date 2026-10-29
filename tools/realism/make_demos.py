# SPDX-License-Identifier: Apache-2.0
"""Writes the ten G-005 demo event files tools/realism/demos/*.json (definitions in plan/GATES.md, G-005).
Event format: see tools/render/main.cpp. Run: python tools/realism/make_demos.py"""
import json
import pathlib

OUT = pathlib.Path(__file__).resolve().parent / "demos"
MF, PP, FF = 76 / 127, 24 / 127, 124 / 127
GAP = 0.005          # legato overlap: the previous note-off comes this long after the next note-on


def on(t, n, v=MF): return {"t": round(t, 4), "type": "noteOn", "note": n, "velocity": round(v, 6)}
def off(t, n): return {"t": round(t, 4), "type": "noteOff", "note": n}
def param(t, name, value): return {"t": round(t, 4), "type": "param", "name": name, "value": value}


def legato(notes, dur, start=0.0, v=MF):
    ev, t = [], start
    for i, n in enumerate(notes):
        ev.append(on(t, n, v))
        if i:
            ev.append(off(t + GAP, notes[i - 1]))
        t += dur
    ev.append(off(t, notes[-1]))
    return ev, t


def write(name, seconds, events, params=None):
    doc = {"seconds": round(seconds, 4), "params": params or {}, "events": events}
    (OUT / f"{name}.json").write_text(json.dumps(doc, indent=1) + "\n")


ev, end = legato(list(range(52, 83)), 0.35)
write("01-chromatic-E3-Bb5", end + 0.6, ev)

ev, end = legato([58, 65, 70, 74, 77, 74, 70, 65, 58], 0.6)
write("02-lip-slurs-open-C4-G5", end + 0.6, ev)

ev = [on(0.0, 58)] + [param(1.0 + 0.05 * j, "overblow", round(j / 80, 4)) for j in range(81)] + [off(5.5, 58)]
write("03-overblow-sweep-written-C4", 6.0, ev)

ev = [on(0.0, 58)] + [param(1.0 + 0.05 * j, "overblow", round(-j / 40, 4)) for j in range(41)] + [off(3.6, 58)]
write("04-underblow-to-pedal", 4.0, ev)

ev = [{"t": 0.0, "type": "keyswitch", "note": 29}]
for i, n in enumerate([53, 60, 65, 68, 72]):
    ev += [on(0.1 + 0.8 * i, n), off(0.1 + 0.8 * i + 0.7, n)]
write("05-fixed-valves-1-3-partials", 0.1 + 0.8 * 5 + 0.4, ev, {"fixedValves": True})

ev, t = [], 0.0
for v in (PP, MF, FF):
    ev += [on(t, 65, v), off(t + 1.5, 65)]
    t += 1.8
write("06-dynamics-pp-mf-ff-written-G4", t + 0.5, ev)

ev, t = [param(0.0, "intonationRealism", 0.0)], 0.0
for r in (0.0, 1.0):
    ev.append(param(t, "intonationRealism", r))
    for n in range(52, 65):
        ev += [on(t, n), off(t + 0.45, n)]
        t += 0.5
write("07-intonation-realism-0-vs-1", t + 0.5, ev)

ev, end = legato([58, 62, 65, 70, 69, 67, 65], 0.5)
ev.append({"t": 1.0, "type": "vibrato", "value": 0.6})
write("08-legato-phrase-vibrato", end + 0.6, ev)

ev, t = [], 0.0
for b in (0.0, 0.5, 1.0):
    ev.append(param(t, "brightness", b))
    for n in (70, 79):
        ev += [on(t, n, FF), off(t + 1.4, n)]
        t += 1.5
write("09-brightness-0-05-1-ff", t + 0.5, ev)

ev, t = [], 0.0
for _ in range(8):                                            # 16th notes at 120 bpm; the off and the next on at one time
    ev += [off(t, 65)] if t > 0 else []
    ev.append(on(t, 65))
    t += 0.125
ev.append(off(t, 65))
t += 0.5
for n in (58, 62, 65, 70):                                    # detached: next note-on 50 ms after the previous note-off
    ev += [on(t, n), off(t + 0.4, n)]
    t += 0.45
t += 0.5 - 0.05
notes = [58, 62, 65, 70]                                      # legato: next on at +0.4, previous off at +0.41
for i, n in enumerate(notes):
    ev.append(on(t + 0.4 * i, n))
    if i:
        ev.append(off(t + 0.4 * i + 0.01, notes[i - 1]))
ev.append(off(t + 0.4 * 4, notes[-1]))
write("10-articulation", t + 0.4 * 4 + 0.6, ev)
print("wrote", len(list(OUT.glob("*.json"))), "demo files")
