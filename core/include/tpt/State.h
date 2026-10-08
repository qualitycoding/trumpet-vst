// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tpt/TrumpetVoice.h"
#include <optional>
#include <string>
#include <string_view>

namespace tpt {

/// Plugin state as JSON (D-015): {"format":"tpt-state-1","params":{<VoiceParameters field names>...}}.
std::string serializeState(const VoiceParameters& p);

/// Inverse of serializeState. Never throws: input > 64 KiB, invalid JSON, wrong "format" -> nullopt; missing or
/// wrongly typed fields keep their defaults; the result is clamped().
std::optional<VoiceParameters> deserializeState(std::string_view json) noexcept;

} // namespace tpt
