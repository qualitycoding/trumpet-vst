// SPDX-License-Identifier: Apache-2.0
#include "tpt/ResonatorTable.h"
#include "tpt/Errors.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <utility>

namespace tpt {

using nlohmann::json;

namespace {
constexpr std::size_t kMaxBytes = 1024 * 1024;
constexpr double kTwoPi = 6.283185307179586476925286766559;

[[noreturn]] void fail(const std::string& what) { throw ParseError("resonator table: " + what); }

const json& field(const json& o, const char* key) {
    if (!o.is_object()) fail(std::string("expected an object around '") + key + "'");
    const auto it = o.find(key);
    if (it == o.end()) fail(std::string("missing '") + key + "'");
    return *it;
}

double number(const json& o, const char* key) {
    const json& v = field(o, key);
    if (!v.is_number()) fail(std::string("'") + key + "' is not a number");
    const double d = v.get<double>();
    if (!std::isfinite(d)) fail(std::string("'") + key + "' is not finite");
    return d;
}

std::string text(const json& o, const char* key) {
    const json& v = field(o, key);
    if (!v.is_string()) fail(std::string("'") + key + "' is not a string");
    return v.get<std::string>();
}

const json& array(const json& o, const char* key) {
    const json& v = field(o, key);
    if (!v.is_array()) fail(std::string("'") + key + "' is not an array");
    return v;
}

std::complex<double> pair(const json& v, const char* what) {
    if (!v.is_array() || v.size() != 2 || !v[0].is_number() || !v[1].is_number()) fail(std::string(what) + " is not a number pair");
    const double a = v[0].get<double>(), b = v[1].get<double>();
    if (!std::isfinite(a) || !std::isfinite(b)) fail(std::string(what) + " is not finite");
    return {a, b};
}

Trigger parseTrigger(const std::string& t) {
    if (t == "-") return Trigger::None;
    if (t == "1") return Trigger::First;
    if (t == "3") return Trigger::Third;
    fail("bad trigger '" + t + "'");
}

RadiationSection::Type parseRadiationType(const std::string& t) {
    using T = RadiationSection::Type;
    if (t == "highpass") return T::HighPass;
    if (t == "lowpass") return T::LowPass;
    if (t == "peak") return T::Peak;
    if (t == "highshelf") return T::HighShelf;
    if (t == "lowshelf") return T::LowShelf;
    fail("bad radiation type '" + t + "'");
}

void check(bool ok, const std::string& what) { if (!ok) fail(what); }
} // namespace

struct ResonatorTable::Data {
    std::vector<BoreState> states;
    std::vector<NoteEntry> notes;
    std::vector<RadiationSection> radiation;
    std::size_t noTriggerState[kValveCombinationCount] = {};
    double nlpLength = 0.0, nlpBeta = 0.0, rho = 0.0, c = 0.0;
};

ResonatorTable ResonatorTable::fromJson(std::string_view jsonText) {
    if (jsonText.size() > kMaxBytes) fail("input larger than 1 MiB");
    try {
        const json j = json::parse(jsonText.begin(), jsonText.end(), nullptr, false);
        if (j.is_discarded()) fail("syntax error");
        if (!j.is_object()) fail("top level is not an object");
        check(text(j, "format") == "tpt-resonators-1", "unsupported format");

        auto d = std::make_shared<Data>();
        const json& air = field(j, "air");
        d->c = number(air, "c");
        d->rho = number(air, "rho");
        check(d->c >= 300.0 && d->c <= 400.0, "air.c out of [300, 400]");
        check(d->rho >= 1.0 && d->rho <= 1.4, "air.rho out of [1.0, 1.4]");
        const json& nlp = field(j, "nlp");
        d->nlpLength = number(nlp, "length_m");
        d->nlpBeta = number(nlp, "beta");
        check(d->nlpLength >= 0.0 && d->nlpLength <= 2.0, "nlp.length_m out of [0, 2]");
        check(d->nlpBeta > 0.0 && d->nlpBeta <= 2.0, "nlp.beta out of (0, 2]");

        const json& rad = array(j, "radiation");
        check(rad.size() >= 1 && rad.size() <= 6, "radiation needs 1-6 sections");
        for (const json& r : rad) {
            RadiationSection s;
            s.type = parseRadiationType(text(r, "type"));
            s.freqHz = number(r, "f_hz");
            s.q = number(r, "q");
            s.gainDb = number(r, "gain_db");
            check(s.freqHz > 10.0 && s.freqHz < 20000.0, "radiation f_hz out of (10, 20000)");
            check(s.q > 0.1 && s.q < 10.0, "radiation q out of (0.1, 10)");
            check(std::fabs(s.gainDb) <= 40.0, "radiation gain out of +-40 dB");
            d->radiation.push_back(s);
        }

        const json& states = array(j, "states");
        check(states.size() >= 8 && states.size() <= 64, "states need 8-64 entries");
        std::set<std::string> ids;
        bool haveNoTrigger[kValveCombinationCount] = {};
        for (const json& sj : states) {
            BoreState st;
            st.id = text(sj, "id");
            check(ids.insert(st.id).second, "duplicate state id '" + st.id + "'");
            const auto v = parseValves(text(sj, "valves"));
            check(v.has_value(), "bad valves name in state '" + st.id + "'");
            st.valves = *v;
            st.trigger = parseTrigger(text(sj, "trigger"));
            if (sj.contains("trigger_len_m")) {
                st.triggerLengthM = number(sj, "trigger_len_m");
                check(st.triggerLengthM >= 0.0 && st.triggerLengthM <= 0.5, "trigger_len_m out of [0, 0.5]");
            }
            const json& modes = array(sj, "modes");
            check(modes.size() >= 4 && modes.size() <= 24, "state '" + st.id + "' needs 4-24 modes");
            for (const json& mj : modes) {
                Mode m;
                m.s = pair(field(mj, "s"), "s");
                m.R = pair(field(mj, "R"), "R");
                check(m.s.real() < 0.0, "Re(s) must be negative");
                const double hz = m.s.imag() / kTwoPi;
                check(hz > 0.0 && hz < 20000.0, "Im(s)/2pi out of (0, 20000) Hz");
                check(m.R.real() > 0.0, "Re(R) must be positive");
                check(std::abs(m.R) < 1e13, "|R| too large");
                check(st.modes.empty() || m.s.imag() > st.modes.back().s.imag(), "modes must be sorted by Im(s)");
                st.modes.push_back(m);
            }
            if (st.trigger == Trigger::None) haveNoTrigger[st.valves] = true;
            d->states.push_back(std::move(st));
        }
        for (int v = 0; v < kValveCombinationCount; ++v) check(haveNoTrigger[v], "missing no-trigger state for a valve combination");
        // index of the first no-trigger state of each combination
        for (int v = 0; v < kValveCombinationCount; ++v)
            for (std::size_t i = 0; i < d->states.size(); ++i)
                if (d->states[i].valves == v && d->states[i].trigger == Trigger::None) { d->noTriggerState[v] = i; break; }

        const json& notes = array(j, "notes");
        check(notes.size() >= 1 && notes.size() <= 128, "notes need 1-128 entries");
        std::set<std::pair<int, bool>> seen;
        for (const json& nj : notes) {
            NoteEntry n;
            const double w = number(nj, "written");
            check(w == std::floor(w) && w >= 54.0 && w <= 89.0, "written out of 54..89");
            n.written = static_cast<int>(w);
            const std::string kind = text(nj, "kind");
            check(kind == "std" || kind == "alt", "bad kind");
            n.alternate = kind == "alt";
            check(seen.insert({n.written, n.alternate}).second, "duplicate note");
            const std::string sid = text(nj, "state");
            const auto it = std::find_if(d->states.begin(), d->states.end(), [&](const BoreState& s) { return s.id == sid; });
            check(it != d->states.end(), "note refers to unknown state '" + sid + "'");
            n.stateIndex = static_cast<int>(it - d->states.begin());
            const double p = number(nj, "partial");
            check(p == std::floor(p) && p >= 1.0 && p <= std::min<double>(13.0, static_cast<double>(it->modes.size())),
                  "partial out of range");
            n.partial = static_cast<int>(p);
            n.fscale = number(nj, "fscale");
            n.naturalDevCents = number(nj, "natural_dev_cents");
            check(n.fscale >= 0.8 && n.fscale <= 1.25, "fscale out of [0.8, 1.25]");
            check(std::fabs(n.naturalDevCents) <= 100.0, "natural_dev_cents out of +-100");
            d->notes.push_back(n);
        }

        ResonatorTable t;
        t.d_ = std::move(d);
        return t;
    } catch (const ParseError&) {
        throw;
    } catch (const std::exception& e) {
        throw ParseError(std::string("resonator table: ") + e.what());
    }
}

std::size_t ResonatorTable::stateCount() const { return d_->states.size(); }

const BoreState& ResonatorTable::state(std::size_t index) const {
    if (index >= d_->states.size()) throw std::out_of_range("ResonatorTable::state");
    return d_->states[index];
}

std::size_t ResonatorTable::stateFor(Valves v) const {
    if (v >= kValveCombinationCount) throw std::out_of_range("ResonatorTable::stateFor");
    return d_->noTriggerState[v];
}

const NoteEntry* ResonatorTable::note(int written, bool alternate) const {
    for (const auto& n : d_->notes)
        if (n.written == written && n.alternate == alternate) return &n;
    return nullptr;
}

const std::vector<NoteEntry>& ResonatorTable::notes() const { return d_->notes; }

double ResonatorTable::partialHz(std::size_t stateIndex, int partial) const {
    const BoreState& s = state(stateIndex);
    if (partial < 1 || static_cast<std::size_t>(partial) > s.modes.size()) throw std::out_of_range("ResonatorTable::partialHz");
    return s.modes[static_cast<std::size_t>(partial) - 1].s.imag() / kTwoPi;
}

const std::vector<RadiationSection>& ResonatorTable::radiation() const { return d_->radiation; }
double ResonatorTable::nlpLengthM() const { return d_->nlpLength; }
double ResonatorTable::nlpBeta() const { return d_->nlpBeta; }
double ResonatorTable::airDensity() const { return d_->rho; }
double ResonatorTable::soundSpeed() const { return d_->c; }

} // namespace tpt
