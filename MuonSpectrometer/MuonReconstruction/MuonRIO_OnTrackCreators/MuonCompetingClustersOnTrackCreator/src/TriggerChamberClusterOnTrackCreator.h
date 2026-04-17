/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_TRIGGERCHAMBERCLUSTERONTRACKCREATOR_H
#define MUON_TRIGGERCHAMBERCLUSTERONTRACKCREATOR_H

#include <list>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonCompetingClustersOnTrackCreator.h"

namespace Track {
class LocalParameters;
class Surface;
}  // namespace Track
namespace Muon {
class MuonClusterOnTrack;

/**
   @brief Tool to cluster several trigger measurements in different gas-gaps of
   the same detector module
*/
class TriggerChamberClusterOnTrackCreator
    : public extends<AthAlgTool, IMuonCompetingClustersOnTrackCreator> {
   public:
    using base_class::base_class;
    virtual ~TriggerChamberClusterOnTrackCreator() = default;

    StatusCode initialize();

    /** method to create a CompetingMuonClustersOnTrack using the PrepRawData
     * hits and a scaled factor for the errors */
    std::unique_ptr<CompetingMuonClustersOnTrack> createBroadCluster(
        const std::list<const Trk::PrepRawData*>&, const double) const;

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusterCreator{
        this, "ClusterCreator",
        "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator"};

    Gaudi::Property<bool> m_chooseBroadestCluster{this, "ChooseBroadestCluster", true};

    // private methods
    void applyClusterConsistency(
        std::list<int>& limitingChannels,
        std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots) const;

    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> createPrdRots(
        const std::list<const Trk::PrepRawData*>& prds) const;

    void makeClustersBySurface(
        std::list<int>& limitingChannels,
        std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots,
        const std::list<const Trk::PrepRawData*>& prds,
        const std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& rots) const;

    void makeOverallParameters(
        Trk::LocalParameters& parameters, Amg::MatrixX& errorMatrix,
        std::unique_ptr<Trk::Surface>& surface,
        std::list<int>& limitingChannels,
        std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots) const;
};

}  // namespace Muon

#endif  // MUON_TRIGGERCHAMBERCLUSTERONTRACKCREATOR_H
