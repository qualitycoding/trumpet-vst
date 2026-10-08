// SPDX-License-Identifier: Apache-2.0
// STUB (D-018): replaced in S-004.
#include "tpt/Analysis.h"
#include "tpt/Errors.h"

namespace tpt {
double estimateF0(const float*, std::size_t, double, double, double) { throw NotImplemented("estimateF0"); }
std::vector<double> harmonicLevelsDb(const float*, std::size_t, double, double, int) { throw NotImplemented("harmonicLevelsDb"); }
double spectralCentroidHz(const float*, std::size_t, double) { throw NotImplemented("spectralCentroidHz"); }
int nearestPartial(double, const std::vector<double>&) { throw NotImplemented("nearestPartial"); }
double rmsDb(const float*, std::size_t) { throw NotImplemented("rmsDb"); }
} // namespace tpt
