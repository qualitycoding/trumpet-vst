// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-012.
#include "tpt/State.h"
#include "tpt/Errors.h"

namespace tpt {
std::string serializeState(const VoiceParameters&) { throw NotImplemented("serializeState"); }
std::optional<VoiceParameters> deserializeState(std::string_view) noexcept { return std::nullopt; }
} // namespace tpt
