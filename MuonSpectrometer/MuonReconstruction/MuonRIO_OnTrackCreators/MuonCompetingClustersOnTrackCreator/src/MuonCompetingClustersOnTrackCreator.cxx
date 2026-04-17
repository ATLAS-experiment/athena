/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCompetingClustersOnTrackCreator.h"

#include <list>
#include <vector>

#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkPrepRawData/PrepRawData.h"

namespace Muon {

StatusCode MuonCompetingClustersOnTrackCreator::initialize() {

    ATH_MSG_VERBOSE("MuonCompetingClustersOnTrackCreator::Initializing");
    ATH_CHECK(m_clusterCreator.retrieve());

    return StatusCode::SUCCESS;
}

std::unique_ptr<CompetingMuonClustersOnTrack>
MuonCompetingClustersOnTrackCreator::createBroadCluster(
    const std::list<const Trk::PrepRawData*>& prds,
    const double errorScaleFactor) const {
    if (prds.empty()) {
        ATH_MSG_WARNING("Empty prepdata list");
        return nullptr;
    }
    // implement cluster formation
    std::vector<std::unique_ptr<const MuonClusterOnTrack>>rios{};;
    auto assocProbs = std::vector<double>();
    const double prob = 1. / (errorScaleFactor * errorScaleFactor);
    for (const Trk::PrepRawData* prd : prds) {
        Identifier id = prd->identify();
        const Trk::TrkDetElementBase* detEl = prd->detectorElement();
        const Amg::Vector3D gHitPos = detEl->center(id);
        std::unique_ptr<const Muon::MuonClusterOnTrack> cluster{m_clusterCreator->createRIO_OnTrack(*prd, gHitPos)};
        if (!cluster) {
            ATH_MSG_WARNING("No cluster..." << (*prd));
            continue;
        }
        rios.push_back(std::move(cluster));
        assocProbs.push_back(prob);
    }
    return std::make_unique<CompetingMuonClustersOnTrack>(
        std::move(rios), std::move(assocProbs));
}
}  // namespace Muon
