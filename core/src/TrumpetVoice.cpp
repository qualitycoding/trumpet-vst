// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-008 .. S-011. Real-time (noexcept) members are inert so that tests fail on assertions.
#include "tpt/TrumpetVoice.h"
#include "tpt/Errors.h"

namespace tpt {
struct TrumpetVoice::Impl {};
VoiceParameters VoiceParameters::clamped() const { throw NotImplemented("VoiceParameters::clamped"); }
TrumpetVoice::TrumpetVoice(std::shared_ptr<const ResonatorTable>) { throw NotImplemented("TrumpetVoice"); }
TrumpetVoice::~TrumpetVoice() = default;
void TrumpetVoice::prepare(double, int) { throw NotImplemented("TrumpetVoice::prepare"); }
void TrumpetVoice::reset() noexcept {}
void TrumpetVoice::setParameters(const VoiceParameters&) noexcept {}
void TrumpetVoice::noteOn(int, float) noexcept {}
void TrumpetVoice::noteOff(int) noexcept {}
void TrumpetVoice::allNotesOff() noexcept {}
void TrumpetVoice::setBreath(float) noexcept {}
void TrumpetVoice::setPitchBend(float) noexcept {}
void TrumpetVoice::setVibratoControl(float) noexcept {}
void TrumpetVoice::setOverblowControl(float) noexcept {}
bool TrumpetVoice::keyswitch(int) noexcept { return false; }
void TrumpetVoice::process(float* out, int n) noexcept { for (int i = 0; i < n; ++i) out[i] = 0.0f; }
int TrumpetVoice::latencySamples() const noexcept { return -1; }
UiState TrumpetVoice::uiState() const noexcept { return {}; }
double TrumpetVoice::soundingHz() const noexcept { return 0.0; }
} // namespace tpt
