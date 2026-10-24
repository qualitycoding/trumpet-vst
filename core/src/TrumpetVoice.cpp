// SPDX-License-Identifier: Apache-2.0
// Real-time monophonic trumpet voice (D-005 .. D-014, D-023): outward-striking lip valve coupled to the modal bore of
// the resonator table, closed-form implicit flow, bilinear modes, oversampling with half-band decimation, nonlinear
// propagation (time warp), radiation biquads, breath noise and the UI/partial tracker.
#include "tpt/TrumpetVoice.h"
#include "BreathNoise.h"
#include "VoiceTuning.h"
#include "tpt/Errors.h"
#include "tpt/Fingering.h"
#include "tpt/LipModel.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace tpt {

namespace {

namespace tu = tuning;
using cd = std::complex<double>;
constexpr double kPi = 3.14159265358979323846;
constexpr int kControlInterval = 16;          // internal samples between control updates (D-006)
constexpr int kMaxModes = 24;
constexpr int kMaxNotes = 16;
constexpr int kTrackRing = 16384;
constexpr int kMaxSections = 6;

double clampd(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); }
double raisedCosineUp(double x) { x = clampd(x, 0.0, 1.0); return 0.5 * (1.0 - std::cos(kPi * x)); }
double lerp(double a, double b, double t) { return a + (b - a) * t; }
double geoLerp(double a, double b, double t) { return a * std::pow(b / a, t); }

float finiteOr(float v, float def) { return std::isfinite(v) ? v : def; }
float clampf(float v, float lo, float hi, float def) { return std::min(hi, std::max(lo, finiteOr(v, def))); }

/// Sustain factor k_sus(L) (D-008).
double sustainFactor(double L) {
    L = clampd(L, 0.0, 1.0);
    if (L <= tu::kSusL0) return tu::kSusLow;
    if (L <= tu::kSusL1) return tu::kSusLow * std::pow(tu::kSusMid / tu::kSusLow, (L - tu::kSusL0) / (tu::kSusL1 - tu::kSusL0));
    return tu::kSusMid * std::pow(tu::kSusHigh / tu::kSusMid, (L - tu::kSusL1) / (tu::kSusL2 - tu::kSusL1));
}

/// Linear blend 0 -> 1 across +-0.05 around a boundary (D-007).
double blend(double L, double boundary) { return clampd((L - (boundary - tu::kBoundaryBlend)) / (2.0 * tu::kBoundaryBlend), 0.0, 1.0); }

/// RBJ cookbook biquad (D-012), direct form II transposed.
struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void design(RadiationSection::Type type, double f, double q, double gainDb, double fs) {
        const double w0 = 2.0 * kPi * std::min(f, 0.45 * fs) / fs, c = std::cos(w0), s = std::sin(w0);
        const double alpha = s / (2.0 * q), A = std::pow(10.0, gainDb / 40.0), sA = std::sqrt(A);
        double B0 = 1, B1 = 0, B2 = 0, A0 = 1, A1 = 0, A2 = 0;
        using T = RadiationSection::Type;
        switch (type) {
        case T::LowPass: B0 = (1 - c) / 2; B1 = 1 - c; B2 = (1 - c) / 2; A0 = 1 + alpha; A1 = -2 * c; A2 = 1 - alpha; break;
        case T::HighPass: B0 = (1 + c) / 2; B1 = -(1 + c); B2 = (1 + c) / 2; A0 = 1 + alpha; A1 = -2 * c; A2 = 1 - alpha; break;
        case T::Peak: B0 = 1 + alpha * A; B1 = -2 * c; B2 = 1 - alpha * A; A0 = 1 + alpha / A; A1 = -2 * c; A2 = 1 - alpha / A; break;
        case T::LowShelf:
            B0 = A * ((A + 1) - (A - 1) * c + 2 * sA * alpha); B1 = 2 * A * ((A - 1) - (A + 1) * c);
            B2 = A * ((A + 1) - (A - 1) * c - 2 * sA * alpha); A0 = (A + 1) + (A - 1) * c + 2 * sA * alpha;
            A1 = -2 * ((A - 1) + (A + 1) * c); A2 = (A + 1) + (A - 1) * c - 2 * sA * alpha; break;
        case T::HighShelf:
            B0 = A * ((A + 1) + (A - 1) * c + 2 * sA * alpha); B1 = -2 * A * ((A - 1) + (A + 1) * c);
            B2 = A * ((A + 1) + (A - 1) * c - 2 * sA * alpha); A0 = (A + 1) - (A - 1) * c + 2 * sA * alpha;
            A1 = 2 * ((A - 1) - (A + 1) * c); A2 = (A + 1) - (A - 1) * c - 2 * sA * alpha; break;
        }
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
        z1 = z2 = 0;
    }
    double process(double x) {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void clear() { z1 = z2 = 0; }
};

double besselI0(double x) {
    double sum = 1.0, term = 1.0;
    for (int k = 1; k < 60; ++k) { term *= (x / (2.0 * k)) * (x / (2.0 * k)); sum += term; }
    return sum;
}

/// 63-tap half-band decimator (Kaiser beta 8), one output per two inputs; delay of 31 input samples.
struct HalfBand {
    static constexpr int N = 63, M = 31, RING = 64;
    std::array<double, M + 1> odd{};   // odd[m] = tap at distance m (m odd), centre in `centre`
    double centre = 0.5;
    std::array<double, RING> buf{};
    int pos = 0;
    bool second = false;
    HalfBand() {
        std::array<double, N> h{};
        double sum = 0;
        for (int n = 0; n < N; ++n) {
            const int m = n - M;
            double ideal = m == 0 ? 0.5 : (m % 2 == 0 ? 0.0 : std::sin(kPi * m / 2.0) / (kPi * m));
            const double r = static_cast<double>(m) / M;
            h[std::size_t(n)] = ideal * besselI0(8.0 * std::sqrt(std::max(0.0, 1.0 - r * r))) / besselI0(8.0);
            sum += h[std::size_t(n)];
        }
        for (auto& v : h) v /= sum;
        centre = h[M];
        for (int m = 1; m <= M; m += 2) odd[std::size_t(m)] = h[std::size_t(M + m)];
    }
    void clear() { buf.fill(0.0); pos = 0; second = false; }
    /// Feeds one input; returns true and sets `out` on every second call.
    bool push(double x, double& out) {
        pos = (pos + 1) & (RING - 1);
        buf[std::size_t(pos)] = x;
        second = !second;
        if (!second) return false;
        // buf[pos - j] = x[n - j]; centre tap at j = M
        double acc = centre * buf[std::size_t((pos - M) & (RING - 1))];
        for (int m = 1; m <= M; m += 2)
            acc += odd[std::size_t(m)] * (buf[std::size_t((pos - M - m) & (RING - 1))] + buf[std::size_t((pos - M + m) & (RING - 1))]);
        out = acc;
        return true;
    }
};

struct NoteSlot { int pitch = 0; int count = 0; float velocity = 0; };

/// What the voice plays for one held note.
struct Setup {
    bool valid = false;
    int concert = 0, written = 0;
    Valves valves = 0;
    Trigger trigger = Trigger::None;
    std::size_t state = 0;
    int partial = 0;
    double fscaleNote = 1.0, devCents = 0.0;
    bool extended = false, fixed = false;
    double floorK = 0.0;
};

/// A lip/pressure/tuning setting that ramps between partials, registers and notes (D-009, D-010).
struct LipSet {
    double flBase = 100, h0 = 1e-4, pth = 1000, floorK = 0;   // flBase: ratio * resonance, unscaled
    double fsNote = 1, devCents = 0;                        // table fscale and natural deviation of the target partial
};

enum class Env { Idle, Attack, Sustain, Release, Stop, Reattack };
enum class Pending { None, FromSilence, Legato, Tongue };

} // namespace

VoiceParameters VoiceParameters::clamped() const {
    VoiceParameters d, o;
    o.brightness = clampf(brightness, 0.f, 1.f, d.brightness);
    o.lipStiffness = clampf(lipStiffness, 0.f, 1.f, d.lipStiffness);
    o.breathNoise = clampf(breathNoise, 0.f, 1.f, d.breathNoise);
    o.vibratoRateHz = clampf(vibratoRateHz, 3.f, 8.f, d.vibratoRateHz);
    o.vibratoDepth = clampf(vibratoDepth, 0.f, 1.f, d.vibratoDepth);
    o.tuningA4Hz = clampf(tuningA4Hz, 415.f, 466.f, d.tuningA4Hz);
    o.intonationRealism = clampf(intonationRealism, 0.f, 1.f, d.intonationRealism);
    o.outputGainDb = clampf(outputGainDb, -24.f, 12.f, d.outputGainDb);
    o.overblow = clampf(overblow, -1.f, 1.f, d.overblow);
    o.useAlternates = useAlternates;
    o.fixedValves = fixedValves;
    return o;
}

struct TrumpetVoice::Impl {
    explicit Impl(std::shared_ptr<const ResonatorTable> t) : table(std::move(t)) {
        (void)fingeringTable();   // construct the compiled-in table now, not on the audio thread
        buildFscaleTables();
        zcIn = table->airDensity() * table->soundSpeed() / (kPi * tu::kCupRadiusM * tu::kCupRadiusM);
        publishUi();
    }

    // ---- configuration ----------------------------------------------------------------------------------------------
    std::shared_ptr<const ResonatorTable> table;
    double fs = 48000, fInt = 96000, T = 1.0 / 96000;
    int os = 2, maxBlock = 512;
    bool prepared = false;
    double zcIn = 400e6 / 1000;

    // D-003 fscale rules, precomputed from the table: exact[valves][partial], meanCombo[valves], meanAll
    std::array<std::array<double, 15>, 8> exactF{};
    std::array<double, 8> meanCombo{};
    double meanAll = 1.0;

    // ---- parameters and controls -------------------------------------------------------------------------------------
    VoiceParameters par;
    double brightS = 0.5, stiffS = 0.5, realismS = 0.0, gainS = 1.0, noiseS = 0.1, bendS = 0.0;
    double bendTarget = 0.0;
    float breath = -1.0f;
    float vibControl = 0.0f;
    float overblowCtl = 0.0f;
    bool overblowOverride = false;
    bool legatoPedal = false;
    Valves held = 0;
    double vibPhase = 0.0;

    // ---- note stack and playing state ----------------------------------------------------------------------------------
    std::array<NoteSlot, kMaxNotes> stack{};
    int stackSize = 0;
    Setup cur;                       // the note being played (top of the stack)
    Pending pending = Pending::None;
    double lastNoteOffTime = -1.0;   // seconds of processed time
    double clock = 0.0;              // processed time, seconds (advanced per output sample)
    bool active = false;             // the voice is running (not idle)
    bool sounded = false;            // the lips were driven since the last idle

    // overblow register
    int regK = 0;
    int target = 0;                  // target partial m
    double residual = 0.0;

    // ---- envelope -----------------------------------------------------------------------------------------------------
    Env env = Env::Idle;
    double envT = 0.0, kOn = 1.0, envFactorAtRelease = 1.0, envFactor = 0.0;
    bool lipClosed = false;
    double pmS = 0.0;                // smoothed pressure target before the envelope
    double panicT = -1.0;            // >= 0: panic fade running
    double panicGain = 1.0;

    // ---- lip / register ramp -------------------------------------------------------------------------------------------
    LipSet from, to, now;
    double rampT = 1e9;              // seconds since the register ramp started
    // ---- valve (mode) ramp ----------------------------------------------------------------------------------------------
    int nm = 14;
    std::array<cd, kMaxModes> fromS{}, fromR{}, toS{}, toR{}, curS{}, curR{};
    double valveT = 1e9;

    // ---- integrator state -------------------------------------------------------------------------------------------
    std::array<double, kMaxModes> aRe{}, aIm{}, gRe{}, gIm{}, qRe{}, qIm{};
    double Z0 = 0, y = 0, v = 0, u = 0, p = 0;
    double flA = 100, flB = 100, h0A = 1e-4, h0B = 1e-4, pmA = 0, pmB = 0;   // segment start/end values
    int ctrlPhase = 0;               // internal samples into the current control segment
    BreathNoise noise;
    double noiseVal = 0.0;

    // ---- output stage -------------------------------------------------------------------------------------------------
    std::vector<double> ringVal, ringTa;
    std::int64_t ringMask = 0, nSrc = 0, ptr = 0;
    int warpD = 0;
    double warpA = 0.0, warpK = 0.0, taPrev = 0.0;
    std::array<Biquad, kMaxSections> bq{};
    int nBq = 0;
    std::array<HalfBand, 2> dec{};
    int nDec = 0;
    std::vector<double> meterRing;
    std::size_t meterPos = 0;
    double meterSum = 0.0;
    double meterDb = -200.0;
    long idleCount = 0;
    std::int64_t idleNeed = 0;

    // ---- tracker ---------------------------------------------------------------------------------------------------
    std::vector<float> trackY;
    std::int64_t trackN = 0;
    double trackHz = 0.0;
    int trackPartial = 0;
    int trackCountdown = 0;
    int latency = 0;

    // ---- UI -------------------------------------------------------------------------------------------------------------
    std::atomic<std::uint64_t> ui{0};
    bool uiSounding = false;
    int uiSoundingPartial = 0;
    std::atomic<double> hzAtomic{0.0};

    // ============================================================================================================
    void buildFscaleTables() {
        exactF.fill({});
        meanCombo.fill(0.0);
        std::array<int, 8> cnt{};
        double sumAll = 0;
        int nAll = 0;
        for (int pass = 0; pass < 2; ++pass) {          // pass 0: standard entries first, pass 1: alternates fill gaps
            for (const NoteEntry& e : table->notes()) {
                if (e.alternate != (pass == 1)) continue;
                const BoreState& st = table->state(std::size_t(e.stateIndex));
                if (pass == 0) { sumAll += e.fscale; ++nAll; }
                if (st.trigger != Trigger::None) continue;
                auto& slot = exactF[st.valves][std::size_t(e.partial)];
                if (slot == 0.0) slot = e.fscale;
            }
        }
        for (const NoteEntry& e : table->notes()) {
            if (e.alternate) { sumAll += e.fscale; ++nAll; }
            const BoreState& st = table->state(std::size_t(e.stateIndex));
            if (st.trigger != Trigger::None) continue;
            meanCombo[st.valves] += e.fscale;
            ++cnt[st.valves];
        }
        meanAll = nAll ? sumAll / nAll : 1.0;
        for (int vv = 0; vv < 8; ++vv) meanCombo[std::size_t(vv)] = cnt[std::size_t(vv)] ? meanCombo[std::size_t(vv)] / cnt[std::size_t(vv)] : 0.0;
    }

    /// fscale of a (valve combination, partial) per D-003.
    double fscaleFor(Valves vv, int partial) const {
        const double e = (partial >= 1 && partial <= 14) ? exactF[vv][std::size_t(partial)] : 0.0;
        if (e > 0.0) return e;
        return meanCombo[vv] > 0.0 ? meanCombo[vv] : meanAll;
    }

    // ============================================================================================================
    void prepare(double sampleRate, int block) {
        if (!(sampleRate >= 22050.0 && sampleRate <= 192000.0)) throw std::invalid_argument("prepare: sample rate out of [22050, 192000]");
        if (block < 1 || block > 8192) throw std::invalid_argument("prepare: block size out of [1, 8192]");
        fs = sampleRate;
        maxBlock = block;
        os = fs >= 88200.0 ? 1 : (fs >= 44100.0 ? 2 : 4);
        fInt = fs * os;
        T = 1.0 / fInt;
        nDec = os == 1 ? 0 : (os == 2 ? 1 : 2);
        // half-band delay: 31 input samples per stage, expressed in output samples (D-006)
        latency = os == 1 ? 0 : static_cast<int>(std::lround(os == 2 ? 31.0 / 2.0 : 31.0 / 4.0 + 31.0 / 2.0));
        // output-stage warp: ring sized from A = beta L_nl,max 30 kPa / (rho c^3) f_int (D-012)
        const double rho = table->airDensity(), c = table->soundSpeed();
        warpA = table->nlpBeta() * table->nlpLengthM() * 2.0 * tu::kWarpClampPa / (rho * c * c * c) * fInt;
        warpD = static_cast<int>(std::ceil(warpA)) + 2;
        std::size_t ringSize = 1;
        while (ringSize < static_cast<std::size_t>(2 * warpD + 8)) ringSize <<= 1;
        ringVal.assign(ringSize, 0.0);
        ringTa.assign(ringSize, 0.0);
        ringMask = static_cast<std::int64_t>(ringSize) - 1;
        meterRing.assign(static_cast<std::size_t>(std::max(1.0, std::round(tu::kMeterWindowS * fs))), 0.0);
        idleNeed = static_cast<std::int64_t>(std::round(tu::kIdleHoldS * fs));
        trackY.assign(kTrackRing, 0.0f);
        noise.setRate(fInt);
        prepared = true;
        resetState(true);
    }

    /// Clears the sound state. `full`: also the controls (reset()), but never held valves (D-026 R-014).
    void resetState(bool full) {
        if (!prepared) return;
        qRe.fill(0); qIm.fill(0);
        y = v = u = p = 0;
        env = Env::Idle; envT = 0; envFactor = 0; lipClosed = false;
        pmS = 0; pmA = pmB = 0;
        panicT = -1; panicGain = 1;
        active = false; sounded = false;
        ctrlPhase = 0;
        noise.reset();
        ringVal.assign(ringVal.size(), 0.0);
        clampCount.store(0, std::memory_order_relaxed);
        nSrc = -1;             // virtual history of zeros whose arrival times equal their indices
        ptr = -warpD - 2;
        for (std::int64_t k = ptr; k <= nSrc; ++k) { ringVal[std::size_t(k & ringMask)] = 0.0; ringTa[std::size_t(k & ringMask)] = double(k); }
        taPrev = double(nSrc);
        for (auto& b : bq) b.clear();
        for (auto& d : dec) d.clear();
        std::fill(meterRing.begin(), meterRing.end(), 0.0);
        meterPos = 0; meterSum = 0; meterDb = -200; idleCount = 0;
        std::fill(trackY.begin(), trackY.end(), 0.0f);
        trackN = 0; trackHz = 0; trackPartial = 0; trackCountdown = 0;
        uiSoundingPartial = 0; uiSounding = false;
        if (full) {
            stackSize = 0; cur = Setup{}; pending = Pending::None;
            breath = -1.0f; bendTarget = 0; bendS = 0; vibControl = 0; vibPhase = 0;
            overblowOverride = false; regK = 0; target = 0; residual = 0;
            lastNoteOffTime = -1.0; clock = 0.0; legatoPedal = false;
            brightS = par.brightness; stiffS = par.lipStiffness; realismS = par.intonationRealism; noiseS = par.breathNoise;
            gainS = std::pow(10.0, par.outputGainDb / 20.0);
        }
        designRadiation();
        publishUi();
    }

    void designRadiation() {
        nBq = 0;
        for (const RadiationSection& s : table->radiation()) {
            if (nBq >= kMaxSections) break;
            bq[std::size_t(nBq++)].design(s.type, s.freqHz, s.q, s.gainDb, fInt);
        }
    }

    // ============================================================================================================
    // Note handling (events are applied at the next control tick; the note stack changes immediately)
    // ============================================================================================================
    std::optional<Setup> resolve(int concert) const {
        const std::optional<Valves> fixed = par.fixedValves ? std::optional<Valves>(held) : std::nullopt;
        const auto r = resolveNote(concert, par.useAlternates, fixed);
        if (!r) return std::nullopt;
        Setup s;
        s.valid = true;
        s.concert = r->concert;
        s.written = r->written;
        s.valves = r->fingering.valves;
        s.partial = r->fingering.partial;
        s.extended = r->extended;
        s.fixed = r->fixedValves;
        if (r->fixedValves) {
            s.state = table->stateFor(s.valves);
            s.trigger = Trigger::None;
            s.fscaleNote = fscaleFor(s.valves, s.partial);
        } else {
            const NoteEntry* e = table->note(r->fingering.written, r->fingering.alternate);
            if (!e) return std::nullopt;
            s.state = std::size_t(e->stateIndex);
            s.trigger = r->fingering.trigger;
            s.fscaleNote = e->fscale;
            s.devCents = e->naturalDevCents;
        }
        if (s.written == 86 || s.written == 87 || s.written == 89) s.floorK = tu::kFloorExtended;
        else if (s.written == 88) s.floorK = tu::kFloorWritten88;
        return s;
    }

    int findPitch(int pitch) const {
        for (int i = 0; i < stackSize; ++i) if (stack[std::size_t(i)].pitch == pitch) return i;
        return -1;
    }

    void noteOn(int concert, float velocity) {
        if (!std::isfinite(velocity) || concert < 0 || concert > 127) return;
        if (velocity <= 0.0f) { noteOff(concert); return; }
        if (concert >= kKeyswitchLow && concert <= kKeyswitchHigh) return;      // keyswitches never sound
        velocity = std::min(velocity, 1.0f);
        lastVel = velocity;
        const auto setup = resolve(concert);
        if (!setup) return;                                                     // out of range: ignored
        const bool othersHeld = stackSize > 0 && !(stackSize == 1 && stack[0].pitch == concert);
        bool legato = othersHeld;
        if (!legato && legatoPedal && active && lastNoteOffTime >= 0.0 && clock - lastNoteOffTime <= tu::kLegatoPedalWindowS) legato = true;
        const bool silent = !active;
        // stack update: one entry per pitch with a count; the most recent note is last
        int i = findPitch(concert);
        if (i >= 0) {
            NoteSlot s = stack[std::size_t(i)];
            ++s.count;
            s.velocity = velocity;
            for (int k = i; k + 1 < stackSize; ++k) stack[std::size_t(k)] = stack[std::size_t(k + 1)];
            stack[std::size_t(stackSize - 1)] = s;
        } else if (stackSize < kMaxNotes) {
            stack[std::size_t(stackSize++)] = NoteSlot{concert, 1, velocity};
        } else {
            for (int k = 0; k + 1 < stackSize; ++k) stack[std::size_t(k)] = stack[std::size_t(k + 1)];
            stack[std::size_t(stackSize - 1)] = NoteSlot{concert, 1, velocity};
        }
        cur = *setup;
        updateOverblowRegister();
        target = targetPartialFor(cur);
        pending = silent ? Pending::FromSilence : (legato ? Pending::Legato : Pending::Tongue);
        if (silent) { active = true; ctrlPhase = 0; }
        publishUi();
    }

    void noteOff(int concert) {
        const int i = findPitch(concert);
        if (i < 0) return;
        if (--stack[std::size_t(i)].count > 0) return;
        for (int k = i; k + 1 < stackSize; ++k) stack[std::size_t(k)] = stack[std::size_t(k + 1)];
        --stackSize;
        if (stackSize > 0) {
            if (i == stackSize) {        // the sounding (top) note ended: fall back to the previous held note, legato
                const auto s = resolve(stack[std::size_t(stackSize - 1)].pitch);
                if (s) { cur = *s; pending = Pending::Legato; }
            }
        } else {
            lastNoteOffTime = clock;
            if (env == Env::Attack || env == Env::Sustain || env == Env::Reattack || env == Env::Stop) {
                envFactorAtRelease = (env == Env::Stop) ? 0.0 : envFactor;
                env = Env::Release;
                envT = 0;
            }
            pending = Pending::None;
            cur.concert = cur.written = 0;
        }
        publishUi();
    }

    void allNotesOff() {
        stackSize = 0;
        pending = Pending::None;
        cur = Setup{};
        if (active) panicT = 0.0;
        lastNoteOffTime = clock;
        publishUi();
    }

    bool keyswitch(int note) {
        const auto vv = keyswitchValves(note);
        if (!vv) return false;
        if (par.fixedValves) {
            held = *vv;
            retargetCurrent();
            publishUi();
        }
        return true;
    }

    /// Re-resolves the sounding note after a change of held valves, alternates or mode (a legato change).
    void retargetCurrent() {
        if (stackSize == 0 || !active) return;
        const auto s = resolve(stack[std::size_t(stackSize - 1)].pitch);
        if (!s) return;
        cur = *s;
        pending = pending == Pending::FromSilence ? Pending::FromSilence : Pending::Legato;
    }

    void setParameters(const VoiceParameters& in) {
        const VoiceParameters p = in.clamped();
        const bool rebuild = p.useAlternates != par.useAlternates || p.fixedValves != par.fixedValves;
        if (p.overblow != par.overblow) overblowOverride = false;
        par = p;
        if (rebuild) retargetCurrent();
        publishUi();
    }

    // ============================================================================================================
    // Control tick
    // ============================================================================================================
    double blowingLevel() const {
        if (breath >= 0.0f) return clampd(breath, 0.0, 1.0);
        return stackSize ? clampd(stack[std::size_t(stackSize - 1)].velocity, 0.0, 1.0) : clampd(lastVel, 0.0, 1.0);
    }

    LipSet lipSetFor(const Setup& s, int m) const {
        const std::size_t mi = static_cast<std::size_t>(std::clamp(m, 1, 13));
        const double fres = table->partialHz(s.state, std::clamp(m, 1, static_cast<int>(table->state(s.state).modes.size())));
        const LipParams lp = lipSettingFor(std::clamp(m, 1, 13), fres);
        LipSet l;
        l.flBase = lp.flHz * tu::kFlTrim[mi];
        l.h0 = lp.h0M * tu::kH0Trim[mi];
        l.pth = thresholdPressurePa(std::clamp(m, 1, 13)) * tu::kPmTrim[mi];
        l.floorK = s.floorK;
        l.devCents = (m == s.partial && !s.fixed) ? s.devCents : 0.0;
        l.fsNote = (m == s.partial) ? s.fscaleNote : fscaleFor(s.valves, m);
        return l;
    }

    void loadModes(std::array<cd, kMaxModes>& S, std::array<cd, kMaxModes>& R, std::size_t stateIdx) const {
        const BoreState& st = table->state(stateIdx);
        const std::size_t n = std::min<std::size_t>(st.modes.size(), kMaxModes);
        for (std::size_t k = 0; k < n; ++k) { S[k] = st.modes[k].s; R[k] = st.modes[k].R; }
    }

    int modeCount(std::size_t stateIdx) const { return static_cast<int>(std::min<std::size_t>(table->state(stateIdx).modes.size(), kMaxModes)); }

    void updateOverblowRegister() {
        const double o = overblowOverride ? static_cast<double>(overblowCtl) : static_cast<double>(par.overblow);
        const double a = 2.0 * clampd(o, -1.0, 1.0);
        const int sa = a > 0 ? 1 : (a < 0 ? -1 : 0), sk = regK > 0 ? 1 : (regK < 0 ? -1 : 0);
        const double b = std::fabs(a);
        if (regK != 0 && sa != 0 && sa != sk) regK = 0;
        int j = std::abs(regK);
        if (b >= j + 1) j = static_cast<int>(std::floor(b));
        else if (b < j - tu::kDownHysteresis) j = static_cast<int>(std::floor(b + tu::kDownHysteresis));
        regK = (sa == 0 ? 0 : sa) * j;
        if (sa == 0) regK = 0;
        residual = a - regK;
    }

    int targetPartialFor(const Setup& s) const {
        const int cap = std::max(s.partial, tu::kCapPartial);
        return std::clamp(s.partial + regK, 1, std::min(cap, 13));
    }

    void startRamp(const Setup& s, int m, bool snapshotFrom) {
        if (snapshotFrom) from = now;
        to = lipSetFor(s, m);
        rampT = 0.0;
    }

    void applyPending(const double dt) {
        (void)dt;
        if (pending == Pending::None) return;
        const Pending kind = pending;
        pending = Pending::None;
        if (!cur.valid) return;
        updateOverblowRegister();
        const int m = targetPartialFor(cur);
        const LipSet next = lipSetFor(cur, m);
        const int newNm = modeCount(cur.state);
        if (kind == Pending::FromSilence) {
            qRe.fill(0); qIm.fill(0);
            nm = newNm;
            loadModes(toS, toR, cur.state);
            fromS = toS; fromR = toR; curS = toS; curR = toR;
            valveT = 1e9;
            from = to = now = next;
            rampT = 1e9;
            target = m;
            const double L = blowingLevel();
            const double kAcc = lerp(tu::kAccLow, tu::kAccHigh, blend(L, tu::kAccBoundaryL));
            const double pmSus = std::max(sustainFactor(L), next.floorK) * next.pth * brighten();
            kOn = std::max(kAcc, tu::kOnsetMinThresholdMul * next.pth / std::max(pmSus, 1e-9));
            y = lerp(tu::kY0Low, tu::kY0High, blend(L, tu::kY0BoundaryL)) * next.h0 * heightScale();
            v = 0; u = 0; p = 0;
            env = Env::Attack; envT = 0; lipClosed = false;
            pmS = pmSus;
            sounded = true; snapSegment = true;
            trackN = 0; trackHz = 0; trackPartial = 0;
        } else if (kind == Pending::Tongue) {
            // new bore state, tuning and lip setting apply at the start of the stop (D-010)
            nm = newNm;
            loadModes(toS, toR, cur.state);
            fromS = toS; fromR = toR; curS = toS; curR = toR;
            valveT = 1e9;
            from = to = now = next;
            rampT = 1e9;
            target = m;
            env = Env::Stop; envT = 0; lipClosed = true; y = 0; v = 0;
            pmS = std::max(sustainFactor(blowingLevel()), next.floorK) * next.pth * brighten();
            sounded = true; snapSegment = true;
        } else { // Legato: lip slur and/or valve slur, no re-attack
            const bool sameState = stateSameAsTarget();
            from = now;
            if (!(env == Env::Attack || env == Env::Sustain || env == Env::Reattack)) {
                // a slur onto a releasing/stopped voice resumes blowing
                env = Env::Sustain; envT = 0; lipClosed = false;
            }
            if (!sameState || newNm != nm) {
                fromS = curS; fromR = curR;
                nm = std::min(nm, newNm);
                loadModes(toS, toR, cur.state);
                valveT = 0.0;
            }
            to = next;
            rampT = 0.0;
            target = m;
        }
        lastStateIdx = cur.state;
        publishUi();
    }

    std::size_t lastStateIdx = 0;
    bool snapSegment = true;
    float lastVel = 0.5f;
    bool stateSameAsTarget() const { return lastStateIdx == cur.state; }

    double brighten() const {
        const double r = residual;
        return r >= 0 ? 1.0 + tu::kBrightenUp * r : 1.0 + tu::kBrightenDown * r;
    }
    double heightScale() const { return tu::kStiffBase - tu::kStiffSlope * stiffS; }

    /// Frequency factor of bend and vibrato (runtime part W) .
    double runtimeScale() const {
        const double depth = std::max<double>(par.vibratoDepth, vibControl);
        const double vib = tu::kVibratoCents * depth * std::sin(vibPhase);
        return std::pow(2.0, bendS / 12.0) * std::pow(2.0, vib / 1200.0);
    }

    void controlTick() {
        const double dt = kControlInterval * T;
        const double alpha = 1.0 - std::exp(-dt / tu::kParamSmoothS);
        // smoothed parameters
        brightS += alpha * (par.brightness - brightS);
        stiffS += alpha * (par.lipStiffness - stiffS);
        realismS += alpha * (par.intonationRealism - realismS);
        noiseS += alpha * (par.breathNoise - noiseS);
        gainS += alpha * (std::pow(10.0, par.outputGainDb / 20.0) - gainS);
        bendS += alpha * (bendTarget - bendS);
        vibPhase += 2.0 * kPi * par.vibratoRateHz * dt;
        if (vibPhase > 2.0 * kPi) vibPhase -= 2.0 * kPi;

        applyPending(dt);

        if (cur.valid && stackSize > 0) {
            updateOverblowRegister();
            const int m = targetPartialFor(cur);
            if (m != target && env != Env::Idle) { from = now; to = lipSetFor(cur, m); rampT = 0.0; target = m; publishUi(); }
        }

        // register ramp and mode ramp progress
        rampT += dt;
        valveT += dt;
        const double rho = clampd(rampT / tu::kRegisterRampS, 0.0, 1.0);
        now.flBase = geoLerp(from.flBase, to.flBase, rho);
        now.h0 = lerp(from.h0, to.h0, rho);
        now.pth = lerp(from.pth, to.pth, rho);
        now.floorK = lerp(from.floorK, to.floorK, rho);
        now.fsNote = geoLerp(from.fsNote, to.fsNote, rho);
        now.devCents = lerp(from.devCents, to.devCents, rho);
        // only the target of a ramp that has not started yet is "to"; a finished ramp makes from == to
        if (rho >= 1.0) from = to;

        // envelope
        envT += dt;
        switch (env) {
        case Env::Attack: {
            if (envT < tu::kAttackRiseS) envFactor = kOn * raisedCosineUp(envT / tu::kAttackRiseS);
            else {
                const double x = (envT - tu::kAttackRiseS) / tu::kAttackRelaxS;
                envFactor = 1.0 + (kOn - 1.0) * (1.0 - raisedCosineUp(x));
                if (x >= 1.0) { env = Env::Sustain; envFactor = 1.0; }
            }
            break;
        }
        case Env::Sustain: envFactor = 1.0; break;
        case Env::Release:
            envFactor = envFactorAtRelease * (1.0 - raisedCosineUp(envT / tu::kReleaseS));
            break;
        case Env::Stop:
            envFactor = 0.0; lipClosed = true;
            if (envT >= tu::kTongueStopS) { env = Env::Reattack; envT = 0; lipClosed = false; }
            break;
        case Env::Reattack:
            envFactor = raisedCosineUp(envT / tu::kReattackS);
            if (envT >= tu::kReattackS) { env = Env::Sustain; envFactor = 1.0; }
            break;
        case Env::Idle: envFactor = 0.0; break;
        }
        if (panicT >= 0.0) {
            panicT += dt;
            panicGain = 1.0 - raisedCosineUp(panicT / tu::kPanicFadeS);
            if (panicT >= tu::kPanicFadeS) { resetState(false); return; }
        }

        // pressure target
        const double L = blowingLevel();
        const double ksus = std::max(sustainFactor(L), now.floorK);
        const double pmTarget = ksus * now.pth * brighten();
        pmS += (1.0 - std::exp(-dt / tu::kPmSmoothS)) * (pmTarget - pmS);

        // segment end values (linear interpolation within the segment)
        const double Wrt = runtimeScale();
        const double S = now.fsNote * std::pow(2.0, realismS * now.devCents / 1200.0) * (static_cast<double>(par.tuningA4Hz) / 440.0);
        const double scale = S * Wrt;
        flA = flB; h0A = h0B; pmA = pmB;
        flB = now.flBase * scale;
        h0B = now.h0 * heightScale();
        pmB = pmS * envFactor;
        if (snapSegment) { flA = flB; h0A = h0B; pmA = pmB; snapSegment = false; }

        // modes: interpolate states (D-010) and scale by the tuning factor
        const double sigma = clampd(valveT / tu::kValveRampS, 0.0, 1.0);
        Z0 = 0;
        for (int k = 0; k < nm; ++k) {
            const std::size_t ki = std::size_t(k);
            if (sigma >= 1.0) { curS[ki] = toS[ki]; curR[ki] = toR[ki]; }
            else { curS[ki] = fromS[ki] + (toS[ki] - fromS[ki]) * sigma; curR[ki] = fromR[ki] + (toR[ki] - fromR[ki]) * sigma; }
            const cd s = curS[ki] * scale, R = curR[ki] * scale;
            const cd den = 1.0 - s * (T / 2.0);
            const cd a = (1.0 + s * (T / 2.0)) / den, g = R * (T / 2.0) / den;
            aRe[ki] = a.real(); aIm[ki] = a.imag(); gRe[ki] = g.real(); gIm[ki] = g.imag();
            Z0 += 2.0 * g.real();
        }

        // output stage constants
        const double rhoAir = table->airDensity(), c = table->soundSpeed();
        const double Lnl = table->nlpLengthM() * 2.0 * brightS;
        warpK = table->nlpBeta() * Lnl / (rhoAir * c * c * c) * fInt;

        // tracker
        if (--trackCountdown <= 0) { trackCountdown = 16; updateTracker(scale); }
    }

    void updateTracker(double scale) {
        const std::int64_t have = std::min<std::int64_t>(trackN, kTrackRing);
        if (!cur.valid && env == Env::Idle) return;
        const double fTarget = (target >= 1 ? table->partialHz(cur.state, std::min(target, nm)) : 100.0) * scale;
        std::int64_t W = std::max<std::int64_t>(1024, static_cast<std::int64_t>(3.0 * fInt / std::max(fTarget, 20.0)));
        W = std::min<std::int64_t>({W, have, kTrackRing - 1});
        if (W < 64) return;
        float lo = 1e30f, hi = -1e30f;
        for (std::int64_t i = 0; i < W; ++i) {
            const float s = trackY[std::size_t((trackN - 1 - i) & (kTrackRing - 1))];
            lo = std::min(lo, s); hi = std::max(hi, s);
        }
        const double pp = static_cast<double>(hi - lo);
        if (pp < 1e-9) return;
        const double mid = 0.5 * (static_cast<double>(hi) + lo), hyst = tu::kTrackerHysteresis * pp;
        bool armed = false;
        int count = 0;
        double first = 0, last = 0;
        double prev = trackY[std::size_t((trackN - W) & (kTrackRing - 1))] - mid;
        for (std::int64_t i = 1; i < W; ++i) {
            const double s = trackY[std::size_t((trackN - W + i) & (kTrackRing - 1))] - mid;
            if (s < -hyst) armed = true;
            if (armed && prev < 0.0 && s >= 0.0) {
                const double t = double(i - 1) + (-prev) / (s - prev);
                if (count == 0) first = t;
                last = t; ++count; armed = false;
            }
            prev = s;
        }
        if (count < 2) return;
        trackHz = fInt / ((last - first) / (count - 1));
        // nearest partial of the current state, tuned by the actual scale
        int best = 0;
        double bd = 1e300;
        for (int k = 1; k <= nm; ++k) {
            const double d = std::fabs(std::log(trackHz / (table->partialHz(cur.state, k) * scale)));
            if (d < bd) { bd = d; best = k; }
        }
        trackPartial = best;
    }

    // ============================================================================================================
    // UI snapshot
    // ============================================================================================================
    static std::uint64_t pack(const UiState& s) {
        std::uint64_t w = 0;
        w |= std::uint64_t(s.sounding);
        w |= std::uint64_t(s.concert & 127) << 1;
        w |= std::uint64_t(s.written & 127) << 8;
        w |= std::uint64_t(s.valves & 7) << 15;
        w |= std::uint64_t(static_cast<std::uint8_t>(s.trigger) & 3) << 18;
        w |= std::uint64_t(s.targetPartial & 31) << 20;
        w |= std::uint64_t(s.soundingPartial & 31) << 25;
        w |= std::uint64_t(s.extended) << 30;
        w |= std::uint64_t(s.fixedValves) << 31;
        return w;
    }
    static UiState unpack(std::uint64_t w) {
        UiState s;
        s.sounding = w & 1;
        s.concert = int((w >> 1) & 127);
        s.written = int((w >> 8) & 127);
        s.valves = static_cast<Valves>((w >> 15) & 7);
        s.trigger = static_cast<Trigger>((w >> 18) & 3);
        s.targetPartial = int((w >> 20) & 31);
        s.soundingPartial = int((w >> 25) & 31);
        s.extended = (w >> 30) & 1;
        s.fixedValves = (w >> 31) & 1;
        return s;
    }

    void publishUi() {
        UiState s;
        s.sounding = uiSounding;
        s.fixedValves = par.fixedValves;
        s.soundingPartial = uiSoundingPartial;
        if (stackSize > 0 && cur.valid) {
            s.concert = cur.concert; s.written = cur.written; s.valves = cur.valves;
            s.trigger = par.intonationRealism < 0.5f ? cur.trigger : Trigger::None;
            s.targetPartial = target > 0 ? target : cur.partial;
            s.extended = cur.extended;
        } else if (par.fixedValves) {
            s.valves = held;
        }
        ui.store(pack(s), std::memory_order_relaxed);
    }

    // ============================================================================================================
    // Rendering
    // ============================================================================================================
    double limiter(double x) const {
        const double a = std::fabs(x);
        if (a <= tu::kLimiterKnee) return x;
        const double k = 1.0 - tu::kLimiterKnee;
        return std::copysign(tu::kLimiterKnee + k * std::tanh((a - tu::kLimiterKnee) / k), x);
    }

    /// One internal sample; returns true and sets `outSample` when an output sample is complete.
    bool step(double& outSample) {
        if (ctrlPhase == 0) controlTick();
        const double fr = static_cast<double>(ctrlPhase + 1) / kControlInterval;
        const double fl = flA + (flB - flA) * fr, H = h0A + (h0B - h0A) * fr, pm = pmA + (pmB - pmA) * fr;
        if (++ctrlPhase == kControlInterval) ctrlPhase = 0;

        const double wl = 2.0 * kPi * fl;
        const double acc = (pm - p) / 9.0 - (wl / 20.0) * v - wl * wl * (y - H);
        v += T * acc;
        y += T * v;
        if (lipClosed) { y = 0; v = 0; }
        const double h = std::max(y, 0.0);
        trackY[std::size_t(trackN++ & (kTrackRing - 1))] = static_cast<float>(y);

        double P0 = 0;
        for (int k = 0; k < nm; ++k) {
            const std::size_t ki = std::size_t(k);
            P0 += aRe[ki] * qRe[ki] - aIm[ki] * qIm[ki] + gRe[ki] * u;
        }
        P0 *= 2.0;
        noiseVal = noise.next();
        const double cflow = 12e-3 * h * std::sqrt(2.0 / tu::kFlowRhoKgM3) * (1.0 + tu::kNoiseFlowGain * noiseS * noiseVal);
        const double D = pm - P0;
        double unew;
        if (cflow <= 0) unew = 0;
        else if (D >= 0) { const double s = 0.5 * (-Z0 * cflow + std::sqrt(Z0 * Z0 * cflow * cflow + 4 * D)); unew = cflow * s; }
        else { const double s = 0.5 * (-Z0 * cflow + std::sqrt(Z0 * Z0 * cflow * cflow - 4 * D)); unew = -cflow * s; }
        for (int k = 0; k < nm; ++k) {
            const std::size_t ki = std::size_t(k);
            const double r = aRe[ki] * qRe[ki] - aIm[ki] * qIm[ki] + gRe[ki] * (unew + u);
            const double i = aRe[ki] * qIm[ki] + aIm[ki] * qRe[ki] + gIm[ki] * (unew + u);
            qRe[ki] = r; qIm[ki] = i;
        }
        u = unew;
        p = P0 + Z0 * u;
        if (!std::isfinite(p) || std::fabs(p) > tu::kSafetyPressurePa) { fault = true; return false; }

        // outgoing wave and nonlinear propagation (D-012)
        const double pplus = 0.5 * (p + zcIn * u);
        const double pc = clampd(pplus, -tu::kWarpClampPa, tu::kWarpClampPa);
        const double off = clampd(warpK * pc, -(warpD - 1.0), warpD - 1.0);
        if (std::fabs(warpK * pc) > warpD - 1.0) clampCount.fetch_add(1, std::memory_order_relaxed);
        ++nSrc;
        const double ta = std::max(taPrev, double(nSrc) - off);
        taPrev = ta;
        ringVal[std::size_t(nSrc & ringMask)] = pplus;
        ringTa[std::size_t(nSrc & ringMask)] = ta;
        const double j = double(nSrc - warpD);
        while (ptr + 1 <= nSrc && ringTa[std::size_t((ptr + 1) & ringMask)] <= j) ++ptr;
        const double t0 = ringTa[std::size_t(ptr & ringMask)], t1 = ringTa[std::size_t((ptr + 1) & ringMask)];
        const double v0 = ringVal[std::size_t(ptr & ringMask)], v1 = ringVal[std::size_t((ptr + 1) & ringMask)];
        double w = (ptr + 1 <= nSrc && t1 > t0) ? v0 + (v1 - v0) * clampd((j - t0) / (t1 - t0), 0.0, 1.0) : v0;
        for (int b = 0; b < nBq; ++b) w = bq[std::size_t(b)].process(w);
        w *= tu::kOutGain * gainS * panicGain;
        w += tu::kNoiseOutGain * noiseS * (pm / std::max(now.pth, 1.0)) * noiseVal * panicGain;

        if (os == 1) { outSample = w; return true; }
        double x = w;
        for (int d = 0; d < nDec; ++d) {
            double o;
            if (!dec[std::size_t(d)].push(x, o)) return false;
            x = o;
        }
        outSample = x;
        return true;
    }

    bool fault = false;
    std::atomic<long> clampCount{0};

    void process(float* out, int n) {
        if (n <= 0) return;
        if (!active) {
            for (int i = 0; i < n; ++i) out[i] = 0.0f;
            clock += n / fs;
            meterDb = -200.0;
            uiSounding = stackSize > 0;
            uiSoundingPartial = 0;
            publishUi();
            hzAtomic.store(0.0, std::memory_order_relaxed);
            return;
        }
        fault = false;
        for (int i = 0; i < n; ++i) {
            double s = 0;
            for (int k = 0; k < os && !fault && active; ++k) step(s);
            if (fault) {
                resetState(false);
                for (int j = i; j < n; ++j) out[j] = 0.0f;
                clock += n / fs;
                publishUi();
                return;
            }
            if (!active) {
                for (int j = i; j < n; ++j) out[j] = 0.0f;
                clock += (n - i) / fs;
                break;
            }
            const double yo = limiter(s);
            out[i] = static_cast<float>(yo);
            // level meter: RMS over the last 20 ms
            const double sq = double(out[i]) * double(out[i]);
            meterSum += sq - meterRing[meterPos];
            meterRing[meterPos] = sq;
            if (++meterPos >= meterRing.size()) meterPos = 0;
            
            clock += 1.0 / fs;
        }
        const double ms = std::max(meterSum, 0.0) / static_cast<double>(meterRing.size());
        meterDb = ms > 1e-30 ? 10.0 * std::log10(ms) : -200.0;
        // idle rule: below -100 dBFS for 50 ms after the release
        const bool released = stackSize == 0 && env == Env::Release && envT > tu::kReleaseS;
        if (released && meterDb < tu::kIdleDb) idleCount += n;
        else idleCount = 0;
        if (idleCount >= idleNeed) {
            resetState(false);
            return;
        }
        uiSounding = stackSize > 0 || meterDb > tu::kSoundingDb;
        uiSoundingPartial = meterDb < tu::kSoundingDb ? 0 : trackPartial;
        hzAtomic.store(meterDb < tu::kSoundingDb ? 0.0 : trackHz, std::memory_order_relaxed);
        publishUi();
    }
};

// ---- public interface ---------------------------------------------------------------------------------------------------
TrumpetVoice::TrumpetVoice(std::shared_ptr<const ResonatorTable> table) {
    if (!table) throw std::invalid_argument("TrumpetVoice: null table");
    impl_ = std::make_unique<Impl>(std::move(table));
}
TrumpetVoice::~TrumpetVoice() = default;
void TrumpetVoice::prepare(double sampleRate, int maxBlock) { impl_->prepare(sampleRate, maxBlock); }
void TrumpetVoice::reset() noexcept { impl_->resetState(true); }
void TrumpetVoice::setParameters(const VoiceParameters& p) noexcept { impl_->setParameters(p); }
void TrumpetVoice::noteOn(int concertMidi, float velocity) noexcept { impl_->noteOn(concertMidi, velocity); }
void TrumpetVoice::noteOff(int concertMidi) noexcept { impl_->noteOff(concertMidi); }
void TrumpetVoice::allNotesOff() noexcept { impl_->allNotesOff(); }
void TrumpetVoice::setBreath(float level) noexcept {
    if (!std::isfinite(level)) return;
    impl_->breath = level < 0.0f ? -1.0f : std::min(level, 1.0f);
}
void TrumpetVoice::setPitchBend(float semitones) noexcept {
    if (!std::isfinite(semitones)) return;
    impl_->bendTarget = clampd(semitones, -2.0, 2.0);
}
void TrumpetVoice::setVibratoControl(float amount) noexcept {
    if (!std::isfinite(amount)) return;
    impl_->vibControl = std::min(1.0f, std::max(0.0f, amount));
}
void TrumpetVoice::setOverblowControl(float overblow) noexcept {
    if (!std::isfinite(overblow)) return;
    impl_->overblowCtl = std::min(1.0f, std::max(-1.0f, overblow));
    impl_->overblowOverride = true;
}
bool TrumpetVoice::keyswitch(int midiNote) noexcept { return impl_->keyswitch(midiNote); }
void TrumpetVoice::process(float* out, int n) noexcept { impl_->process(out, n); }
int TrumpetVoice::latencySamples() const noexcept { return impl_->latency; }
UiState TrumpetVoice::uiState() const noexcept { return Impl::unpack(impl_->ui.load(std::memory_order_relaxed)); }
double TrumpetVoice::soundingHz() const noexcept { return impl_->hzAtomic.load(std::memory_order_relaxed); }
void TrumpetVoice::setHeldValves(Valves v) noexcept { impl_->held = static_cast<Valves>(v & 7); impl_->publishUi(); }
void TrumpetVoice::setLegatoPedal(bool on) noexcept { impl_->legatoPedal = on; }
void TrumpetVoice::releaseOverblowControl() noexcept { impl_->overblowOverride = false; }
Valves TrumpetVoice::heldValves() const noexcept { return impl_->held; }
long TrumpetVoice::warpClampCount() const noexcept { return impl_->clampCount.load(std::memory_order_relaxed); }

} // namespace tpt
