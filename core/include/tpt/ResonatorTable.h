// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/Valves.h"
#include <complex>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tpt {

/// One complex mode of the input impedance: q' = s q + R u, mouthpiece pressure p = 2 Re(sum q) (Freour 2022 eq. 1,
/// with R = Zc C_k). s in rad/s (Re < 0), R in Pa s^-1 per m^3/s.
struct Mode {
    std::complex<double> s;
    std::complex<double> R;
};

/// One bore state = one valve combination, optionally with a slide trigger extended (D-004).
struct BoreState {
    std::string id;                 ///< "0", "13", ... or "<valves>+t<1|3>@<written>" for trigger variants
    Valves valves = 0;
    Trigger trigger = Trigger::None;
    double triggerLengthM = 0.0;
    std::vector<Mode> modes;        ///< sorted by Im(s); mode k (1-based) <-> partial k
};

/// Per-fingering data (D-004, D-011): which bore state it plays on, its tuning scale and natural deviation.
struct NoteEntry {
    int written = 0;
    bool alternate = false;
    int stateIndex = 0;
    int partial = 0;
    double fscale = 1.0;            ///< calibrated frequency scale (tpt_calibrate), in [0.8, 1.25]
    double naturalDevCents = 0.0;   ///< deviation from 12-TET at Intonation realism = 1, in [-100, 100]
};

/// Radiation filter section (RBJ cookbook biquad designed at run time for the actual sample rate, D-012).
struct RadiationSection {
    enum class Type { HighPass, LowPass, Peak, HighShelf, LowShelf } type = Type::HighPass;
    double freqHz = 1000.0, q = 0.707, gainDb = 0.0;
};

/// Immutable resonator table: the file format "tpt-resonators-1" of data/trumpet_resonators.json (D-004).
class ResonatorTable {
public:
    /// Parses and validates; every failure (syntax, missing field, any limit of D-004) throws ParseError. The size
    /// check (<= 1 MiB) happens before parsing.
    static ResonatorTable fromJson(std::string_view json);

    std::size_t stateCount() const;
    const BoreState& state(std::size_t index) const;             ///< throws std::out_of_range
    /// Index of the state for a valve combination without trigger (always present after validation).
    std::size_t stateFor(Valves v) const;
    /// Entry for a fingering, or nullptr if the table has none.
    const NoteEntry* note(int written, bool alternate) const;
    const std::vector<NoteEntry>& notes() const;
    /// Resonance frequency (Hz) of partial k (1-based) of a state: Im(s_k) / (2 pi). Throws std::out_of_range.
    double partialHz(std::size_t stateIndex, int partial) const;
    const std::vector<RadiationSection>& radiation() const;
    double nlpLengthM() const;     ///< equivalent cylinder length for nonlinear propagation (m)
    double nlpBeta() const;        ///< (gamma + 1) / 2
    double airDensity() const;     ///< kg/m^3
    double soundSpeed() const;     ///< m/s

private:
    struct Data;
    std::shared_ptr<const Data> d_;
};

/// The table embedded into the binary at build time from data/trumpet_resonators.json (D-004). Null-terminated.
const char* embeddedResonatorJson();

} // namespace tpt
