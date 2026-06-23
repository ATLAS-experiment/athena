/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MLMSTRACKSEEDER_H
#define MUONTRACKFINDINGTOOLS_MLMSTRACKSEEDER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"

#include <memory>
#include <string>

namespace MuonR4 {

class MlMsTrackSeeder : public AthMessaging {
public:
  struct Config {
    MsTrackSeeder::Config baselineSeeder{};
    std::string candidateDecoration{"trackCandidateIds"};
    unsigned int minSegmentsPerCandidate{2};
    bool fallbackToBaselineIfUndecorated{true};
    bool fallbackToBaselineIfNoCandidates{false};
    bool runCandidatesInParallel{true};
  };

  MlMsTrackSeeder(const std::string& msgName, Config&& cfg);

  const MsTrackSeeder& baselineSeeder() const { return m_baselineSeeder; }

  std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                       const ActsTrk::GeometryContext& gctx,
                                                       const xAOD::MuonSegmentContainer& segments) const;

private:
  Config m_cfg{};
  MsTrackSeeder m_baselineSeeder;
};

} // namespace MuonR4

#endif
