// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/TrumpetVoice.h"
#include <array>
#include <string>

namespace tpt {

/// Axis-aligned rectangle in normalised view coordinates: x, y in [0, 1], origin top-left (D-013).
struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    bool intersects(const Rect& o) const { return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h; }
    bool inside01() const { return x >= 0 && y >= 0 && x + w <= 1 && y + h <= 1 && w > 0 && h > 0; }
};

inline constexpr int kLadderRungs = 13;   ///< partials 1..13 (Fingering.h kMaxPartial)

/// Geometry of the trumpet drawing (D-013): side view, bell to the right, mouthpiece to the left.
struct TrumpetLayout {
    std::array<Rect, 3> valveCap;      ///< finger buttons of valves 1, 2, 3 (left to right)
    std::array<Rect, 3> valveCasing;   ///< valve casings below the caps
    Rect firstTrigger;                 ///< first-valve slide trigger (drawn extended when shown)
    Rect thirdTrigger;                 ///< third-valve slide ring (drawn extended when shown)
    std::array<Rect, kLadderRungs> ladder;    ///< harmonic ladder rungs, partial 1 at the bottom
    Rect caption;
    Rect instrument;                   ///< bounding box of the body drawing
};

const TrumpetLayout& trumpetLayout();

/// Caption under the drawing (D-013), e.g. "Written D4 · Concert C4 · Valves 1-3 · Partial 3 · 3rd slide".
/// Parts: "Written <name>", "Concert <name>", "Valves <\"open\" for 0, else digits joined by '-'>", "Partial <n>"
/// (target partial; if the sounding partial differs and is non-zero: "Partial <target> (sounding <s>)"), then
/// " · 1st slide" / " · 3rd slide" when a trigger is shown, " · extended" when extended, " · fixed valves" when in
/// Fixed-valves mode. Separator is " · " (UTF-8). Idle (concert == 0): "Fixed valves <valves>" in Fixed-valves
/// mode, otherwise the empty string.
std::string captionText(const UiState& s);

} // namespace tpt
