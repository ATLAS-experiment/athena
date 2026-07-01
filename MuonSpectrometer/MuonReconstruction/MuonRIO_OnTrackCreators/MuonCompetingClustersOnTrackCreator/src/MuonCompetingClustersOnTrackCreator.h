/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Interface for MuonDriftCircleOnTrack production
// (for MDT technology)
///////////////////////////////////////////////////////////////////

#ifndef MUON_MUONCOMPETINGCLUSTERSONTRACKCREATOR_H
#define MUON_MUONCOMPETINGCLUSTERSONTRACKCREATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonCompetingClustersOnTrackCreator.h"
#include "TrkEventPrimitives/LocalParameters.h"

namespace Muon {
/**
   @brief Tool to create MuonCompetingClustersOnTrack objects
*/
class MuonCompetingClustersOnTrackCreator
    : public extends<AthAlgTool, IMuonCompetingClustersOnTrackCreator> {
   public:
    using base_class::base_class;
    virtual ~MuonCompetingClustersOnTrackCreator() = default;
    virtual StatusCode initialize();

    /** method to create a CompetingMuonClustersOnTrack using the PrepRawData
     * hits and a scaled factor for the errors */
    std::unique_ptr<CompetingMuonClustersOnTrack> createBroadCluster(
        const std::list<const Trk::PrepRawData*>&,
        const double errorScaleFactor) const;

   private:
    ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusterCreator{
        this, "ClusterCreator",
        "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator",
        "pointer to muon cluster rio ontrack creator"};
};

}  // namespace Muon

#endif
