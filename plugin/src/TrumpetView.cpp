// SPDX-License-Identifier: Apache-2.0
// Programmatic drawing of the trumpet (juce::Path only, no images; D-013).
#include "TrumpetView.h"
#include "tpt/Valves.h"
#include <string>

namespace {
const juce::Colour kAmber(0xFFFFB000), kGrey(0xFF9AA0A6), kBackground(0xFF1B1E23), kPanel(0xFF23272E),
                   kBrass(0xFFB88A2E), kBrassDark(0xFF7A5A18), kSteel(0xFFBFC5CC), kSteelDark(0xFF6C737B), kText(0xFFE6E8EB);

juce::Rectangle<float> toPixels(const tpt::Rect& r, juce::Rectangle<float> area) {
    return {area.getX() + r.x * area.getWidth(), area.getY() + r.y * area.getHeight(), r.w * area.getWidth(), r.h * area.getHeight()};
}
} // namespace

TrumpetView::TrumpetView() { setOpaque(true); }

void TrumpetView::setState(const tpt::UiState& s) {
    if (s == state_) return;
    state_ = s;
    repaint();
}

void TrumpetView::paint(juce::Graphics& g) {
    const auto& L = tpt::trumpetLayout();
    const auto area = getLocalBounds().toFloat();
    g.fillAll(kBackground);

    const auto box = toPixels(L.instrument, area);
    g.setColour(kPanel);
    g.fillRoundedRectangle(box, 8.0f);

    // --- tubing (brass): lead pipe through the valve block, the bell, and the tuning slide loop below the casings ---
    auto pt = [&](float x, float y) { return juce::Point<float>(area.getX() + x * area.getWidth(), area.getY() + y * area.getHeight()); };
    const float tube = 0.018f * area.getHeight() * 2.2f;
    juce::Path lead;
    lead.startNewSubPath(pt(0.05f, 0.40f));
    lead.lineTo(pt(0.60f, 0.40f));
    g.setColour(kBrassDark);
    g.strokePath(lead, juce::PathStrokeType(tube + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(kBrass);
    g.strokePath(lead, juce::PathStrokeType(tube, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path loop;                                       // main tuning slide loop below the valve casings
    loop.startNewSubPath(pt(0.33f, 0.40f));
    loop.cubicTo(pt(0.20f, 0.45f), pt(0.20f, 0.70f), pt(0.40f, 0.70f));
    loop.lineTo(pt(0.52f, 0.70f));
    loop.cubicTo(pt(0.66f, 0.70f), pt(0.66f, 0.45f), pt(0.58f, 0.40f));
    g.setColour(kBrassDark);
    g.strokePath(loop, juce::PathStrokeType(tube * 0.7f + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(kBrass);
    g.strokePath(loop, juce::PathStrokeType(tube * 0.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path bell;                                       // flaring bell, bell to the right
    bell.startNewSubPath(pt(0.60f, 0.40f - 0.02f));
    bell.cubicTo(pt(0.68f, 0.38f), pt(0.74f, 0.30f), pt(0.82f, 0.14f));
    bell.lineTo(pt(0.82f, 0.66f));
    bell.cubicTo(pt(0.74f, 0.50f), pt(0.68f, 0.42f), pt(0.60f, 0.40f + 0.02f));
    bell.closeSubPath();
    g.setColour(kBrass);
    g.fillPath(bell);
    g.setColour(kBrassDark);
    g.strokePath(bell, juce::PathStrokeType(2.0f));

    juce::Path mouthpiece;                                 // mouthpiece cup at the far left
    mouthpiece.startNewSubPath(pt(0.02f, 0.37f));
    mouthpiece.lineTo(pt(0.06f, 0.385f));
    mouthpiece.lineTo(pt(0.06f, 0.415f));
    mouthpiece.lineTo(pt(0.02f, 0.43f));
    mouthpiece.closeSubPath();
    g.setColour(kSteel);
    g.fillPath(mouthpiece);

    // --- slide triggers: grey when idle, amber and drawn extended by 0.03 when shown (D-013) ---
    auto drawTrigger = [&](const tpt::Rect& r, bool shown, bool extendLeft) {
        tpt::Rect q = r;
        if (shown) {
            if (extendLeft) { q.x -= 0.03f; q.w += 0.03f; } else { q.w += 0.03f; }
        }
        const auto rc = toPixels(q, area);
        g.setColour(shown ? kAmber : kGrey);
        g.fillRoundedRectangle(rc, 4.0f);
        g.setColour(kSteelDark);
        g.drawRoundedRectangle(rc, 4.0f, 1.5f);
    };
    drawTrigger(L.firstTrigger, state_.trigger == tpt::Trigger::First, true);
    drawTrigger(L.thirdTrigger, state_.trigger == tpt::Trigger::Third, false);

    // --- valve casings and finger buttons (pressed: amber, pushed down) ---
    const tpt::Valves valves = state_.valves;
    for (int i = 0; i < 3; ++i) {
        const auto casing = toPixels(L.valveCasing[std::size_t(i)], area);
        g.setColour(kSteel);
        g.fillRoundedRectangle(casing, 5.0f);
        g.setColour(kSteelDark);
        g.drawRoundedRectangle(casing, 5.0f, 1.5f);
        auto cap = toPixels(L.valveCap[std::size_t(i)], area);
        const bool pressed = (valves & (1 << i)) != 0;
        if (pressed) cap = cap.translated(0.0f, 0.4f * cap.getHeight());
        g.setColour(pressed ? kAmber : kGrey);
        g.fillRoundedRectangle(cap, 5.0f);
        g.setColour(kSteelDark);
        g.drawRoundedRectangle(cap, 5.0f, 1.5f);
        g.setColour(juce::Colours::black.withAlpha(0.75f));
        g.setFont(juce::Font(juce::FontOptions(14.0f)));
        g.drawText(juce::String(i + 1), casing.withHeight(casing.getHeight() * 0.4f).translated(0.0f, casing.getHeight() * 0.3f),
                   juce::Justification::centred, false);
    }

    // --- harmonic ladder: target rung outlined in amber, sounding rung filled amber ---
    for (int k = 1; k <= tpt::kLadderRungs; ++k) {
        const auto rung = toPixels(L.ladder[std::size_t(k - 1)], area);
        const bool target = state_.targetPartial == k, sounding = state_.soundingPartial == k;
        if (sounding) {
            g.setColour(kAmber);
            g.fillRoundedRectangle(rung, 3.0f);
        } else {
            g.setColour(kPanel);
            g.fillRoundedRectangle(rung, 3.0f);
        }
        g.setColour(target ? kAmber : kGrey.withAlpha(0.45f));
        g.drawRoundedRectangle(rung, 3.0f, target ? 3.0f : 1.0f);
        g.setColour(sounding ? juce::Colours::black : kText);
        g.setFont(juce::Font(juce::FontOptions(13.0f)));
        g.drawText("p" + juce::String(k), rung, juce::Justification::centred, false);
    }

    // --- sounding lamp and caption (drawn inside layout.caption, D-013) ---
    g.setColour(state_.sounding ? kAmber : kGrey.withAlpha(0.4f));
    g.fillEllipse(area.getX() + 0.025f * area.getWidth(), area.getY() + 0.03f * area.getHeight(), 14.0f, 14.0f);
    g.setColour(kText);
    g.setFont(juce::Font(juce::FontOptions(16.0f)));
    g.drawText(juce::String::fromUTF8(tpt::captionText(state_).c_str()), toPixels(L.caption, area).toNearestInt(),
               juce::Justification::centredLeft, true);
}
