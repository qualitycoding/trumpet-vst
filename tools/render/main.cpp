// SPDX-License-Identifier: Apache-2.0
// tpt_render (S-015, D-017): offline renderer of the core voice to a mono float32 WAV.
//   tpt_render --print-tuning
//   tpt_render --note <concert> --velocity <0..1> --seconds <s> --fs <hz> --out <file.wav> [--param <name>=<value> ...]
//   tpt_render --events <file.json> --fs <hz> --out <file.wav>
// Exit code 0 on success, 2 on bad arguments or a malformed event file (no output file is left behind).
#include "VoiceTuning.h"
#include "tpt/ResonatorTable.h"
#include "tpt/TrumpetVoice.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

struct UsageError : std::runtime_error { using std::runtime_error::runtime_error; };

bool toBool(const json& v) {
    if (v.is_boolean()) return v.get<bool>();
    if (v.is_number()) return v.get<double>() != 0.0;
    throw UsageError("expected a boolean");
}

double number(const json& v) {
    if (!v.is_number()) throw UsageError("expected a number");
    return v.get<double>();
}

void setParam(tpt::VoiceParameters& p, const std::string& name, const json& v) {
    if (name == "brightness") p.brightness = static_cast<float>(number(v));
    else if (name == "lipStiffness") p.lipStiffness = static_cast<float>(number(v));
    else if (name == "breathNoise") p.breathNoise = static_cast<float>(number(v));
    else if (name == "vibratoRateHz") p.vibratoRateHz = static_cast<float>(number(v));
    else if (name == "vibratoDepth") p.vibratoDepth = static_cast<float>(number(v));
    else if (name == "tuningA4Hz") p.tuningA4Hz = static_cast<float>(number(v));
    else if (name == "intonationRealism") p.intonationRealism = static_cast<float>(number(v));
    else if (name == "outputGainDb") p.outputGainDb = static_cast<float>(number(v));
    else if (name == "overblow") p.overblow = static_cast<float>(number(v));
    else if (name == "useAlternates") p.useAlternates = toBool(v);
    else if (name == "fixedValves") p.fixedValves = toBool(v);
    else throw UsageError("unknown parameter '" + name + "'");
}

json parseValue(const std::string& s) {
    if (s == "true") return true;
    if (s == "false") return false;
    char* end = nullptr;
    const double d = std::strtod(s.c_str(), &end);
    if (end == s.c_str() || *end != '\0') throw UsageError("bad value '" + s + "'");
    return d;
}

struct Event {
    double t = 0;
    std::string type;
    json body;
};

void put32(std::string& s, std::uint32_t v) { for (int i = 0; i < 4; ++i) s.push_back(static_cast<char>((v >> (8 * i)) & 0xFF)); }
void put16(std::string& s, std::uint16_t v) { for (int i = 0; i < 2; ++i) s.push_back(static_cast<char>((v >> (8 * i)) & 0xFF)); }

void writeWav(const std::string& path, const std::vector<float>& x, int fs) {
    std::string h;
    const std::uint32_t dataBytes = static_cast<std::uint32_t>(x.size() * 4);
    h += "RIFF"; put32(h, 36 + 12 + dataBytes); h += "WAVE";
    h += "fmt "; put32(h, 16); put16(h, 3); put16(h, 1); put32(h, static_cast<std::uint32_t>(fs)); put32(h, static_cast<std::uint32_t>(fs) * 4); put16(h, 4); put16(h, 32);
    h += "fact"; put32(h, 4); put32(h, static_cast<std::uint32_t>(x.size()));
    h += "data"; put32(h, dataBytes);
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out) throw std::runtime_error("cannot write " + tmp);
        out.write(h.data(), static_cast<std::streamsize>(h.size()));
        out.write(reinterpret_cast<const char*>(x.data()), static_cast<std::streamsize>(dataBytes));
        if (!out) throw std::runtime_error("write failed");
    }
    std::filesystem::rename(tmp, path);
}

std::unique_ptr<tpt::TrumpetVoice> makeVoice(double fs, const tpt::VoiceParameters& p) {
    auto table = std::make_shared<const tpt::ResonatorTable>(tpt::ResonatorTable::fromJson(tpt::embeddedResonatorJson()));
    auto v = std::make_unique<tpt::TrumpetVoice>(table);
    v->prepare(fs, 512);
    v->setParameters(p);
    return v;
}

void renderTo(tpt::TrumpetVoice& v, std::vector<float>& out, std::size_t from, std::size_t to) {
    while (from < to) {
        const int n = static_cast<int>(std::min<std::size_t>(512, to - from));
        v.process(out.data() + from, n);
        from += static_cast<std::size_t>(n);
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::string out, eventsPath;
        double note = -1, velocity = 0.6, seconds = 2.0, fs = 48000.0;
        bool haveNote = false, printTuning = false;
        std::vector<std::pair<std::string, std::string>> params;
        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];
            auto next = [&]() -> std::string { if (i + 1 >= argc) throw UsageError("missing value after " + a); return argv[++i]; };
            if (a == "--print-tuning") printTuning = true;
            else if (a == "--note") { note = parseValue(next()).get<double>(); haveNote = true; }
            else if (a == "--velocity") velocity = parseValue(next()).get<double>();
            else if (a == "--seconds") seconds = parseValue(next()).get<double>();
            else if (a == "--fs") fs = parseValue(next()).get<double>();
            else if (a == "--out") out = next();
            else if (a == "--events") eventsPath = next();
            else if (a == "--param") {
                const std::string kv = next();
                const auto eq = kv.find('=');
                if (eq == std::string::npos) throw UsageError("--param needs name=value");
                params.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
            } else throw UsageError("unknown argument " + a);
        }
        if (printTuning) { std::cout << tpt::tuning::toJson() << "\n"; return 0; }
        if (out.empty()) throw UsageError("--out is required");
        if (haveNote == !eventsPath.empty()) throw UsageError("give exactly one of --note and --events");
        if (!(fs >= 22050.0 && fs <= 192000.0)) throw UsageError("--fs out of [22050, 192000]");

        tpt::VoiceParameters p;
        std::vector<Event> events;
        if (haveNote) {
            if (!(seconds > 0) || !(velocity > 0 && velocity <= 1.0) || note < 0 || note > 127) throw UsageError("bad --note/--velocity/--seconds");
            for (const auto& kv : params) setParam(p, kv.first, parseValue(kv.second));
            events.push_back({0.0, "noteOn", json{{"note", static_cast<int>(note)}, {"velocity", velocity}}});
        } else {
            std::ifstream in(eventsPath);
            if (!in) throw UsageError("cannot read " + eventsPath);
            const json j = json::parse(in, nullptr, false);
            if (j.is_discarded() || !j.is_object() || !j.contains("seconds") || !j.contains("events") || !j["events"].is_array())
                throw UsageError("malformed event file");
            seconds = number(j["seconds"]);
            if (!(seconds > 0)) throw UsageError("bad seconds");
            if (j.contains("params")) {
                if (!j["params"].is_object()) throw UsageError("params must be an object");
                for (auto it = j["params"].begin(); it != j["params"].end(); ++it) setParam(p, it.key(), it.value());
            }
            for (const json& e : j["events"]) {
                if (!e.is_object() || !e.contains("t") || !e.contains("type") || !e["type"].is_string()) throw UsageError("malformed event");
                events.push_back({number(e["t"]), e["type"].get<std::string>(), e});
            }
            for (const Event& e : events) {   // validate the types and arguments before any audio is produced
                const std::string& t = e.type;
                if (t == "noteOn") { number(e.body.at("note")); number(e.body.at("velocity")); }
                else if (t == "noteOff" || t == "keyswitch") number(e.body.at("note"));
                else if (t == "param") { if (!e.body.contains("name") || !e.body.contains("value")) throw UsageError("param event needs name and value"); tpt::VoiceParameters probe; setParam(probe, e.body["name"].get<std::string>(), e.body["value"]); }
                else if (t == "breath" || t == "bend" || t == "vibrato" || t == "overblowCC") number(e.body.at("value"));
                else if (t == "legatoPedal") toBool(e.body.at("value"));
                else if (t == "allNotesOff") {}
                else throw UsageError("unknown event type '" + t + "'");
            }
        }
        std::stable_sort(events.begin(), events.end(), [](const Event& a, const Event& b) { return a.t < b.t; });

        auto voice = makeVoice(fs, p);
        const std::size_t total = static_cast<std::size_t>(std::llround(seconds * fs));
        std::vector<float> audio(total, 0.0f);
        std::size_t pos = 0;
        for (const Event& e : events) {
            const std::size_t at = std::min<std::size_t>(total, static_cast<std::size_t>(std::max(0.0, std::ceil(e.t * fs - 1e-9))));
            renderTo(*voice, audio, pos, at);
            pos = std::max(pos, at);
            const std::string& t = e.type;
            if (t == "noteOn") voice->noteOn(e.body["note"].get<int>(), static_cast<float>(number(e.body["velocity"])));
            else if (t == "noteOff") voice->noteOff(e.body["note"].get<int>());
            else if (t == "keyswitch") voice->keyswitch(e.body["note"].get<int>());
            else if (t == "param") { setParam(p, e.body["name"].get<std::string>(), e.body["value"]); voice->setParameters(p); }
            else if (t == "breath") voice->setBreath(static_cast<float>(number(e.body["value"])));
            else if (t == "bend") voice->setPitchBend(static_cast<float>(number(e.body["value"])));
            else if (t == "vibrato") voice->setVibratoControl(static_cast<float>(number(e.body["value"])));
            else if (t == "overblowCC") voice->setOverblowControl(static_cast<float>(number(e.body["value"])));
            else if (t == "legatoPedal") voice->setLegatoPedal(toBool(e.body["value"]));
            else if (t == "allNotesOff") voice->allNotesOff();
        }
        renderTo(*voice, audio, pos, total);
        writeWav(out, audio, static_cast<int>(fs));
        return 0;
    } catch (const UsageError& e) {
        std::cerr << "tpt_render: " << e.what() << "\n";
        return 2;
    } catch (const json::exception& e) {
        std::cerr << "tpt_render: malformed event file: " << e.what() << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "tpt_render: error: " << e.what() << "\n";
        return 1;
    }
}
