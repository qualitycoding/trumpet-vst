// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/ResonatorTable.h"
#include "tpt/Valves.h"
#include <memory>

namespace tpt {

/// User parameters (D-016). Ranges are enforced by clamped().
struct VoiceParameters {
    float brightness = 0.5f;          ///< [0, 1]  nonlinear-propagation length x (2 brightness) (D-012)
    float lipStiffness = 0.5f;        ///< [0, 1]  lip opening scale 1.15 - 0.3 s (D-005)
    float breathNoise = 0.1f;         ///< [0, 1]  turbulence noise on the flow (D-012)
    float vibratoRateHz = 5.5f;       ///< [3, 8]
    float vibratoDepth = 0.0f;        ///< [0, 1]  1 = +-25 cents (D-014)
    float tuningA4Hz = 440.0f;        ///< [415, 466]
    float intonationRealism = 0.0f;   ///< [0, 1]  0 = in tune, 1 = natural tendencies (D-011)
    float outputGainDb = 0.0f;        ///< [-24, 12]
    float overblow = 0.0f;            ///< [-1, 1] (D-009)
    bool useAlternates = false;       ///< alternate fingerings (D-002)
    bool fixedValves = false;         ///< Fixed-valves mode (D-003)
    /// Copy with every field clamped to its range; non-finite floats are replaced by the default.
    VoiceParameters clamped() const;
};

/// Snapshot for the UI (D-013). Written by the audio thread through atomics, read lock-free by uiState().
struct UiState {
    bool sounding = false;            ///< a note is held or still ringing above -60 dBFS
    int concert = 0;                  ///< resolved concert note (0 when idle)
    int written = 0;
    Valves valves = 0;                ///< valves pressed (Fixed-valves mode: held valves even when idle)
    Trigger trigger = Trigger::None;  ///< trigger shown: fingering trigger if intonationRealism < 0.5, else None
    int targetPartial = 0;            ///< partial aimed at (fingering partial + overblow register offset)
    int soundingPartial = 0;          ///< partial measured from the oscillation (0 = not oscillating)
    bool extended = false;
    bool fixedValves = false;
    bool operator==(const UiState&) const = default;
};

/// Monophonic real-time trumpet voice (D-005 .. D-014). All methods except the constructor, prepare() and
/// setTable-free queries are real-time safe: no allocation, no locks, no exceptions.
class TrumpetVoice {
public:
    /// Throws std::invalid_argument if table is null.
    explicit TrumpetVoice(std::shared_ptr<const ResonatorTable> table);
    ~TrumpetVoice();
    TrumpetVoice(const TrumpetVoice&) = delete;
    TrumpetVoice& operator=(const TrumpetVoice&) = delete;

    /// Allocates for sampleRate in [22050, 192000] and blocks up to maxBlock (1..8192); throws
    /// std::invalid_argument otherwise. Resets all state.
    void prepare(double sampleRate, int maxBlock);
    void reset() noexcept;

    void setParameters(const VoiceParameters& p) noexcept;   ///< takes p.clamped()
    void noteOn(int concertMidi, float velocity) noexcept;   ///< velocity in (0, 1]; <= 0 behaves as noteOff
    void noteOff(int concertMidi) noexcept;
    void allNotesOff() noexcept;
    /// Breath controller (CC2/CC11) in [0, 1]; once a value >= 0 has been received it replaces velocity as the
    /// blowing level until reset(). A negative value means "no breath controller".
    void setBreath(float level) noexcept;
    void setPitchBend(float semitones) noexcept;              ///< clamped to [-2, 2]
    void setVibratoControl(float amount) noexcept;           ///< aftertouch / mod wheel, [0, 1]
    void setOverblowControl(float overblow) noexcept;        ///< MIDI CC 16 mapping, [-1, 1]; overrides the parameter
                                                             ///< until the parameter value changes
    /// Keyswitch note (MIDI 24..31): in Fixed-valves mode selects the held valves; returns true if the note is a
    /// keyswitch (it never sounds), false otherwise.
    bool keyswitch(int midiNote) noexcept;

    /// Renders n mono samples (n <= maxBlock) into out (overwrites).
    void process(float* out, int n) noexcept;

    /// Decimation-filter latency in output samples (constant after prepare()).
    int latencySamples() const noexcept;
    UiState uiState() const noexcept;
    /// Frequency (Hz) of the current oscillation as tracked by the voice (0 when silent).
    double soundingHz() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tpt
