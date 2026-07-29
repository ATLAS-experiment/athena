/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0TRUTHVALIDATION_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0TRUTHVALIDATION_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/StatusCode.h"
#include "GeneratorObjects/McEventCollection.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"

#include <array>
#include <cstddef>

namespace Trk {
class IExtrapolator;
}

namespace L0Muon {
namespace TgcL0Floating {

struct TruthValidationConfig {
  /// Nominal station-plane z positions used only for validation extrapolation.
  /// These discs are not chamber active-area surfaces.
  double minPt{2000.0};
  double minAbsEta{0.95};
  double maxAbsEta{2.50};
  int requiredStatus{1};
  int maxAbsBarcode{9999};
  double m1AbsZ{13436.5};
  double m2AbsZ{14728.2};
  double m3AbsZ{15148.2};
  double minR{0.0};
  double maxR{11977.0};
  float maxMeanDeltaR{0.08F};
};

struct TruthValidationSummary {
  std::size_t nSelectedTruthMuons{0};
  std::size_t nFullyExtrapolatedTruthMuons{0};
  std::size_t nTruthMuonsWithMatchedCandidate{0};
  std::size_t nUnmatchedCandidates{0};
  /// Position-station-mask distribution of the nearest matched candidate.
  std::array<std::size_t, 8> nNearestMatchedByPositionMask{};
};

/** @brief Truth-only validation of final candidates on nominal station planes.
 *
 * This validation does not establish intersection with a particular chamber
 * active area and is never used by reconstruction or candidate selection.
 */
class TruthValidation {
 public:
  explicit TruthValidation(
      TruthValidationConfig config = TruthValidationConfig{});

  StatusCode validate(const McEventCollection& truthEvents,
                      const Trk::IExtrapolator& extrapolator,
                      const TgcL0CandidateContainer& candidates,
                      const EventContext& ctx,
                      TruthValidationSummary& summary) const;

 private:
  TruthValidationConfig m_config{};
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
