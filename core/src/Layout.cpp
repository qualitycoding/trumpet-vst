// SPDX-License-Identifier: Apache-2.0
#include "tpt/Layout.h"
#include "tpt/Pitch.h"

namespace tpt {

namespace {
TrumpetLayout makeLayout() {
    TrumpetLayout l;
    const float x[3] = {0.36f, 0.44f, 0.52f};
    for (int i = 0; i < 3; ++i) {
        l.valveCap[std::size_t(i)] = {x[i], 0.18f, 0.05f, 0.08f};
        l.valveCasing[std::size_t(i)] = {x[i], 0.28f, 0.05f, 0.24f};
    }
    l.firstTrigger = {0.27f, 0.30f, 0.05f, 0.05f};
    l.thirdTrigger = {0.58f, 0.55f, 0.05f, 0.05f};
    for (int k = 1; k <= kLadderRungs; ++k) l.ladder[std::size_t(k - 1)] = {0.86f, 0.90f - 0.06f * float(k - 1), 0.12f, 0.05f};
    l.caption = {0.02f, 0.92f, 0.80f, 0.07f};
    l.instrument = {0.02f, 0.10f, 0.80f, 0.70f};
    return l;
}

std::string valvesText(Valves v) {
    if (v == 0) return "open";
    std::string s;
    for (int bit = 0; bit < 3; ++bit)
        if (v & (1 << bit)) s += (s.empty() ? "" : "-") + std::to_string(bit + 1);
    return s;
}
} // namespace

const TrumpetLayout& trumpetLayout() {
    static const TrumpetLayout layout = makeLayout();
    return layout;
}

std::string captionText(const UiState& s) {
    if (s.concert == 0) return s.fixedValves ? "Fixed valves " + valvesText(s.valves) : std::string();
    const std::string dot = " \xc2\xb7 ";
    std::string t = "Written " + noteName(s.written) + dot + "Concert " + noteName(s.concert) + dot + "Valves " + valvesText(s.valves) + dot +
                    "Partial " + std::to_string(s.targetPartial);
    if (s.soundingPartial != 0 && s.soundingPartial != s.targetPartial) t += " (sounding " + std::to_string(s.soundingPartial) + ")";
    if (s.trigger == Trigger::First) t += dot + "1st slide";
    if (s.trigger == Trigger::Third) t += dot + "3rd slide";
    if (s.extended) t += dot + "extended";
    if (s.fixedValves) t += dot + "fixed valves";
    return t;
}

} // namespace tpt
