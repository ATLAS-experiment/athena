/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINIDNGALGS_MSTRACKFINIDNGALG_H
#define MUONTRACKFINIDNGALGS_MSTRACKFINIDNGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"


#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "MuonRecToolInterfacesR4/ITrackVisualizationTool.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonR4{
    class MSTrackFindingAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
        
            virtual ~MSTrackFindingAlg();
            /** @brief Standard algorithm hook to setup the extrapolator, retrieve the
             *         tools and declare algorithm's data dependencies */
            virtual StatusCode initialize() override final;
            /** @brief Standard algorithm execution hook */
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Iterates over the search tree and combines close-by segments to a track seed.
             *         Seeds with the same segments as other seeds are deduplicated
             *  @brief ctx: The event's context to access StoreGate & Conditions
             *  @param segments: Full segment container */
            std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                                 const xAOD::MuonSegmentContainer& segments) const;

            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKeyArray<xAOD::MuonSegmentContainer> m_segmentKeys{this, "SegmentContainer", {} };
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Pointer to the MuonDetectorManager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            
            /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
            SG::WriteHandleKey<MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};
            /** @brief Fetch the detector alignment */
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief  Fetch the magnetic field */
            SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_magFieldKey{this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
            /** @brief Segment selection tool to pick the good quality segments */
            ToolHandle<ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Track extrapolation tool */
            ToolHandle<IActsExtrapolationTool> m_extrapolator{this, "Extrapolator" ,"" };
            /** @brief Visualization tool to debug the track finding */
            ToolHandle<MuonValR4::ITrackVisualizationTool> m_visualizationTool{this, "VisualizationTool", ""};
            /** @brief Maximum search window to search segments for */
            Gaudi::Property<double> m_seedHalfLength{this, "SeedHalfLength", 50.*Gaudi::Units::cm};
    };      
}

#endif