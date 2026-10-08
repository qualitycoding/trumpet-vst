// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-003.
#include "tpt/Fingering.h"
#include "tpt/Errors.h"

namespace tpt {
const std::vector<Fingering>& fingeringTable() { throw NotImplemented("fingeringTable"); }
std::optional<Fingering> standardFingering(int) { throw NotImplemented("standardFingering"); }
std::optional<Fingering> alternateFingering(int) { throw NotImplemented("alternateFingering"); }
std::optional<ResolvedNote> resolveNote(int, bool, std::optional<Valves>) { throw NotImplemented("resolveNote"); }
} // namespace tpt
