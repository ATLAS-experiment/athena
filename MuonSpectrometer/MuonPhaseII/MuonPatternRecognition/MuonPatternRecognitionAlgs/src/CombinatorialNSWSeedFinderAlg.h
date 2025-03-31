/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNITIONALGS_COMBINATORIALNSWSEEDFINDERALG_H
#define MUONR4_MUONPATTERNRECOGNITIONALGS_COMBINATORIALNSWSEEDFINDERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <MuonSpacePoint/SpacePointContainer.h>
#include <MuonPatternEvent/MuonPatternContainer.h>

#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>

#include <span>
#include <vector>
 

namespace MuonR4{

using HitVec = SpacePointPerLayerSplitter::HitVec;
using HitLayVec = SpacePointPerLayerSplitter::HitLayVec;
using HitLaySpan = std::span<const HitVec,std::dynamic_extent>;

enum class HitWindow{
  tooLow = 0,

  inside,

  tooHigh
};

class CombinatorialNSWSeedFinderAlg : public AthReentrantAlgorithm {
  
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual ~CombinatorialNSWSeedFinderAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;    

    private:

        /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
        /// Failure is returned in cases, of non-empty keys and failed retrieval
        template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;


        // read handle key for the input maxima (from a previous eta-transform)
        SG::ReadHandleKey<EtaHoughMaxContainer> m_etaKey{this, "CombinatorialReadKey", "MuonHoughNswMaxima"};

         // write handle key for the otuput 
        SG::WriteHandleKey<SegmentSeedContainer> m_writeKey{this, "CombinatorialPhiWriteKey", "MuonHoughNswSegmentSeeds"};

        // access to the ACTS geometry context 
        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        // access to the Muon Id Helper
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        // access to the Muon Detector Manager
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

        std::unique_ptr<SegmentSeed> buildSegmentSeed(HitVec& hits, const AmgSymMatrix(2)& bMatrix, const HoughMaximum& max, 
                                                      const HitLayVec& extensionLayers) const;

        //extend the seed with compatilbe hits using extrapolation to the layers
        HitVec extendHits(const Amg::Vector3D& startPos, 
                        const Amg::Vector3D& direction, 
                        const HitLayVec& stripHitsLayers) const;

        //build and return seeds from the same eta maximum
        std::vector<std::unique_ptr<SegmentSeed>> findSeedsFromMaximum(const HoughMaximum& max, const ActsGeometryContext& gctx) const;

        HitLayVec findCombinatoricHits(const Amg::Vector3D& beamSpot,
                                       const HitLayVec& combinatoricLayers) const;
        
        HitWindow findHitInWindow(const Amg::Vector3D& startPos, 
                                  const SpacePoint* testHit, 
                                  const Amg::Vector3D& dirEstUp,
                                  const Amg::Vector3D& dirEstDn) const;
    
        //the window in theta to search for hits in the seed extension
        DoubleProperty m_windowTheta {this, "m_windowTheta", 0.5 * Gaudi::Units::deg};
        
        //apply a cut threshold in the pulls during the hit extension
        DoubleProperty m_minPullThreshold{this, "m_minPullThreshold", 5.};
        
        /// Pattern visualization tool
        ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};





};

}

#endif
