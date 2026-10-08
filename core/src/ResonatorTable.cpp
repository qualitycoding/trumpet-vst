// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-006.
#include "tpt/ResonatorTable.h"
#include "tpt/Errors.h"

namespace tpt {
struct ResonatorTable::Data {};
ResonatorTable ResonatorTable::fromJson(std::string_view) { throw NotImplemented("ResonatorTable::fromJson"); }
std::size_t ResonatorTable::stateCount() const { throw NotImplemented("stateCount"); }
const BoreState& ResonatorTable::state(std::size_t) const { throw NotImplemented("state"); }
std::size_t ResonatorTable::stateFor(Valves) const { throw NotImplemented("stateFor"); }
const NoteEntry* ResonatorTable::note(int, bool) const { throw NotImplemented("note"); }
const std::vector<NoteEntry>& ResonatorTable::notes() const { throw NotImplemented("notes"); }
double ResonatorTable::partialHz(std::size_t, int) const { throw NotImplemented("partialHz"); }
const std::vector<RadiationSection>& ResonatorTable::radiation() const { throw NotImplemented("radiation"); }
double ResonatorTable::nlpLengthM() const { throw NotImplemented("nlpLengthM"); }
double ResonatorTable::nlpBeta() const { throw NotImplemented("nlpBeta"); }
double ResonatorTable::airDensity() const { throw NotImplemented("airDensity"); }
double ResonatorTable::soundSpeed() const { throw NotImplemented("soundSpeed"); }
// Replaced in S-006 by a definition generated from data/trumpet_resonators.json (core/CMakeLists.txt).
const char* embeddedResonatorJson() { return ""; }
} // namespace tpt
