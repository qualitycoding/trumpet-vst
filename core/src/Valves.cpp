// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-003.
#include "tpt/Valves.h"
#include "tpt/Errors.h"

namespace tpt {
int loweringSemitones(Valves) { throw NotImplemented("loweringSemitones"); }
std::string valvesName(Valves) { throw NotImplemented("valvesName"); }
std::optional<Valves> parseValves(std::string_view) { throw NotImplemented("parseValves"); }
std::optional<Valves> keyswitchValves(int) { throw NotImplemented("keyswitchValves"); }
} // namespace tpt
