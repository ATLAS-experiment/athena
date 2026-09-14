/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_TRUTHTRACKSEEDERTOOL_H
#define MUONTRACKFINDINGTOOLS_TRUTHTRACKSEEDERTOOL_H


#include "GeoPrimitives/GeoPrimitives.h"
///
#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include  "ActsEvent/ContextUtility.h"


namespace MuonR4 {
/** @brief The TruthTrackSeederTool combines the Segments based on their common 
 *         truthParticleLink. If multiple segments in the same chamber share the
 *         truthParticle, then the one which has the most amount of hits is chosen
 *         The initial track parameters are predicted from the closest truth segment
 *         along the trajectory. The tool only works on MC and is also entirely meant
 *         to evaluate the performance of the global chi2 fitter. */
class TruthTrackSeederTool: public extends<AthAlgTool, ITrackSeedingTool> {
        public:
            /** @brief Copy the constructor from the base class */
            using base_class::base_class;
            /**  @copydoc AthAlgTool::initialize  */
            virtual StatusCode initialize() override final;
            /** @copydoc ITrackSeedingTool::findTrackSeeds */
            virtual StatusCode findTrackSeeds(const EventContext& ctx,
                                              std::vector<MsTrackSeed>& outSeeds) const override final;
            /** @copydoc ITrackSeedingTool::estimateStartParameters */
            virtual Acts::Result<Acts::BoundTrackParameters> 
                                estimateStartParameters(const EventContext& ctx,
                                                        const MsTrackSeed& seed) const override final;
            /** @copydoc ITrackSeedingTool::estimateQtimesP */
            virtual double estimateQtimesP(const EventContext& ctx,
                                           const Amg::Vector3D& planeNorm,
                                           std::span<const PosMomPair_t> circlePoints) const override final; 
        private:
            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentContainer", "MuonSegmentsFromR4" };
            /** @brief Dependency on the truth particle link */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthLinkKey{this, "TruthLinkKey", m_segmentKey, "truthParticleLink"};
            /** @brief Instance to the muon detector manager */
            MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Context utility to retrieve the geometry context */
            ActsTrk::ContextUtility m_ctxProvider{this};
    };
}
#endif