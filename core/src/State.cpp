// SPDX-License-Identifier: Apache-2.0
#include "tpt/State.h"
#include <nlohmann/json.hpp>
#include <cmath>

namespace tpt {

using nlohmann::json;

namespace {
constexpr std::size_t kMaxStateBytes = 64 * 1024;
constexpr int kMaxDepth = 16;

/// Rejects absurdly nested input before the parser sees it (strings are skipped).
bool depthOk(std::string_view s) {
    int depth = 0;
    bool inString = false, escape = false;
    for (char c : s) {
        if (inString) {
            if (escape) escape = false;
            else if (c == '\\') escape = true;
            else if (c == '"') inString = false;
        } else if (c == '"') inString = true;
        else if (c == '{' || c == '[') { if (++depth > kMaxDepth) return false; }
        else if (c == '}' || c == ']') --depth;
    }
    return true;
}

void readFloat(const json& params, const char* key, float& out) {
    const auto it = params.find(key);
    if (it != params.end() && it->is_number()) {
        const double d = it->get<double>();
        if (std::isfinite(d)) out = static_cast<float>(d);
    }
}

void readBool(const json& params, const char* key, bool& out) {
    const auto it = params.find(key);
    if (it != params.end() && it->is_boolean()) out = it->get<bool>();
}
} // namespace

std::string serializeState(const VoiceParameters& p) {
    json j;
    j["format"] = "tpt-state-1";
    json& q = j["params"];
    q["brightness"] = p.brightness;
    q["lipStiffness"] = p.lipStiffness;
    q["breathNoise"] = p.breathNoise;
    q["vibratoRateHz"] = p.vibratoRateHz;
    q["vibratoDepth"] = p.vibratoDepth;
    q["tuningA4Hz"] = p.tuningA4Hz;
    q["intonationRealism"] = p.intonationRealism;
    q["outputGainDb"] = p.outputGainDb;
    q["overblow"] = p.overblow;
    q["useAlternates"] = p.useAlternates;
    q["fixedValves"] = p.fixedValves;
    return j.dump();
}

std::optional<VoiceParameters> deserializeState(std::string_view text) noexcept {
    try {
        if (text.size() > kMaxStateBytes || !depthOk(text)) return std::nullopt;
        const json j = json::parse(text.begin(), text.end(), nullptr, false);
        if (j.is_discarded() || !j.is_object()) return std::nullopt;
        const auto f = j.find("format");
        if (f == j.end() || !f->is_string() || f->get<std::string>() != "tpt-state-1") return std::nullopt;
        const auto pi = j.find("params");
        if (pi == j.end() || !pi->is_object()) return std::nullopt;
        VoiceParameters p;
        readFloat(*pi, "brightness", p.brightness);
        readFloat(*pi, "lipStiffness", p.lipStiffness);
        readFloat(*pi, "breathNoise", p.breathNoise);
        readFloat(*pi, "vibratoRateHz", p.vibratoRateHz);
        readFloat(*pi, "vibratoDepth", p.vibratoDepth);
        readFloat(*pi, "tuningA4Hz", p.tuningA4Hz);
        readFloat(*pi, "intonationRealism", p.intonationRealism);
        readFloat(*pi, "outputGainDb", p.outputGainDb);
        readFloat(*pi, "overblow", p.overblow);
        readBool(*pi, "useAlternates", p.useAlternates);
        readBool(*pi, "fixedValves", p.fixedValves);
        return p.clamped();
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace tpt
