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
#include "MuonDetDescrUtils/MuonSectorMapping.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/KDTree.hpp"



#include "TCanvas.h"

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

            virtual StatusCode finalize() override final;
        private:
            /** @brief Use a KDTree as underlying search container */
            using SortTree_t = Acts::KDTree<3, const xAOD::MuonSegment*, double, std::array, 6>;
            using TreeDataVec_t = std::vector<SortTree_t::pair_t>; 
            /** @brief Retrieves the segment container from store gate and fills them into the search tree container
             *  @param ctx: EventContext to ease the store gate access */
            SortTree_t constructTree(const EventContext& ctx) const;

            /** @brief Enum defining the projection plane. Either barrel cylinder or endcap disc */
            using Location = MsTrackSeed::Location;
            /** @brief Fill a segment into the KDTree data vector by projecting it either onto the fictive 
             *         barrel cylinder or onto one of the two endcap discs. If the projection is within the
             *         cylinder / disc boundaries the segment is added. Further, segments at the discontinuous sectors
             *         1 or 16 are also filled into the sectors 17 and 0 to allow for an sector overlap search also in these sectors
             *  @param seg: Pointer to the segment to fill
             *  @param project: Projection plane of choice (Barrel / Endcap)
             *  @param target: Output KDTree data vector to which the successful candidate is appended. */
            void fillInSegment(const xAOD::MuonSegment* seg, 
                               Location project, 
                               TreeDataVec_t& target) const;

            /** @brief Iterates over the search tree and combines close-by segments to a track seed.
             *         Seeds with the same segments as other seeds are deduplicated
             *  @brief ctx: The event's context to access StoreGate & Conditions
             *  @param segSearchTree: Presorted collection of segments */
            std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                                 const SortTree_t& segSearchTree) const;
            
            StatusCode drawEvent(const EventContext& ctx,
                                 const SortTree_t& segSearchTree,
                                 const MsTrackSeedContainer& trackSeeds) const;

            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKeyArray<xAOD::MuonSegmentContainer> m_segmentKeys{this, "SegmentContainer", {} };
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Pointer to the MuonDetectorManager */
            MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            
            /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
            SG::WriteHandleKey<MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};
           
            /** @brief Key to the truth segment selection to draw the segment parameters */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegkey", "TruthSegmentsR4"};
            /** @brief Fetch the detector alignment */
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief  Fetch the magnetic field */
            SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_magFieldKey{this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
            /** @brief Segment selection tool to pick the good quality segments */
            ToolHandle<ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Track extrapolation tool */
            ToolHandle<IActsExtrapolationTool> m_extrapolator{this, "Extrapolator" ,"" };
                
            Gaudi::Property<double> m_seedHalfLength{this, "SeedHalfLength", 50.*Gaudi::Units::cm};
            /** @brief Radius of the barrel reference cylinder onto which all segments are projected */
            double m_refBarrelR{7.*Gaudi::Units::m};
            /** @brief Position along the beam axis of the referece disc onto which all endcap segments are projected */
            double m_refEndcapDiscZ{15.*Gaudi::Units::m};
            /** @brief Radius of the reference disc. */
            double m_refEndcapDiscR{12.*Gaudi::Units::m};

            const Muon::MuonSectorMapping m_sectorMap{};
            mutable std::unique_ptr<TCanvas> m_summaryCan ATLAS_THREAD_SAFE;
            mutable std::atomic<unsigned int> m_canvCounter ATLAS_THREAD_SAFE{0};
    };      
}

#endif