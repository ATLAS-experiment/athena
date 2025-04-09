/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNITIONALGS_PHIHOUGHTRANSFORMALG__H
#define MUONR4_MUONPATTERNRECOGNITIONALGS_PHIHOUGHTRANSFORMALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <MuonSpacePoint/SpacePointContainer.h>
#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonPatternEvent/HoughEventData.h>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPatternEvent/MuonHoughDefs.h"

// muon includes


namespace MuonR4{
    
    /// @brief Algorithm to handle the phi hough transform 
    /// 
    /// This algorithm is responsible for extending existing hough 
    /// maxima found in a previous eta-transform. 
    /// It will try to form a phi extension from the phi-sensitive
    /// measurements attached to the maximum and remove incompatible
    /// ones. It will write MuonSegmentSeeds into the event store
    /// for downstream use. 
    class PhiHoughTransformAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~PhiHoughTransformAlg() = default;
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /// @brief prepare the hough plane once per event. 
            /// Books the accumulator and attaches it to the event data
            /// @param data: Event data object 
            /// @return a status code 
            void prepareHoughPlane(HoughEventData & data) const; 

            /// @brief pre-processing for a given input eta-maximum
            /// Counts potential phi-hits and defines the search space 
            /// @param gctx: The geometry context to fetch the global positions
            /// @param maximum: An (eta) maximum 
            /// @param data: Event data object  
            /// @return a status code 
            void preProcessMaximum(const ActsGeometryContext& gctx,
                                   const MuonR4::HoughMaximum & maximum,
                                   HoughEventData & data) const; 

            /// @brief extend an eta maximum with just a single attached phi measurement. 
            /// Uses the beam spot direction to guess an approximate phi-intercept and direction. 
            /// @param data: event data object
            /// @param maximum: eta-maximum to extend
            /// @return a SegmentSeed extended using the pointing assumption 
            std::unique_ptr<SegmentSeed> recoverSinglePhiMax(HoughEventData & data, const MuonR4::HoughMaximum & maximum) const; 
            
            /// @brief perform a hough search for the most promising phi extension of an eta-maximum
            /// Performs a local hough transform using the phi-measurements on the maximum and returns
            /// the maxima ranked by their compatibility with the eta-measurements on the maximum. 
            /// @param ctx: EventContext to access StoreGate
            /// @param data: event data object
            /// @param maximum: eta maximum to extend
            /// @return: The best maxima found by the extension (lowest number of eta measurements that would need to be discarded)  
            std::vector<MuonR4::ActsPeakFinderForMuon::Maximum> findRankedSegmentSeeds(const EventContext& ctx,
                                                                                       HoughEventData & data, 
                                                                                       const MuonR4::HoughMaximum & maximum) const; 

            /// @brief helper to count the number of eta measurements that would be discarded for a given 
            /// phi extension candidate. Correct extensions should have a very small or zero number.
            /// @param phiMaximum Phi-maximum to evaluate
            /// @param etaMaximum Eta-maximum the candidate was obtained from  
            int countIncompatibleEtaHits(const MuonR4::ActsPeakFinderForMuon::Maximum & phiMaximum, const MuonR4::HoughMaximum &  etaMaximum) const; 
            
            /// @brief constructs a segment seed from an eta maximum and a phi-extension. 
            /// Will keep all pure eta-measurements on the original maximum and add those
            /// eta-phi or pure phi measurements compatible with the extension. 
            /// @param etaMax: the eta-maximum
            /// @param phiMax: the phi-transform maximum 
            /// @return a SegmentSeed representing the updated candidate
            std::unique_ptr<SegmentSeed> buildSegmentSeed(const HoughMaximum & etaMax,  
                                                          const MuonR4::ActsPeakFinderForMuon::Maximum & phiMax) const; 

            // read handle key for the input maxima (from a previous eta-transform)
            SG::ReadHandleKey<EtaHoughMaxContainer> m_maxima{this, "ReadKey", "MuonHoughStationMaxima"};
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<SegmentSeedContainer> m_segmentSeeds{this, "WriteKey", "MuonHoughStationSegmentSeeds"};

            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /// Pattern visualization tool
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
            // steers the target resolution in tan(phi) 
            DoubleProperty m_targetResoTanPhi{this, "ResolutionTargetTanAngle", 0.04};
            // steers the target resolution in the x-axis intercept
            DoubleProperty m_targetResoIntercept{this, "ResolutionTargetIntercept", 30.};
            // minimum search window half width, tan(phi) 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchTanPhi{this, "minSigmasSearchTanPhi", 1.};
            // minimum search window half width, intercept 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchIntercept{this, "minSigmasSearchIntercept", 1.};
            
            // number of accumulator bins in tan(phi)
            // target resolution in the angle
            IntegerProperty m_nBinsTanPhi{this, "nBinsTanAngle", 5};
            // number of accumulator bins in the x-axis intercept
            IntegerProperty m_nBinsIntercept{this, "nBinsIntercept", 10};
            // maximum number of eta measurements allowed to be discarded by 
            // a valid phi-extension
            IntegerProperty m_maxEtaHolesOnMax{this, "maxEtaHoles", 1};
            // flag to steer whether to recover maxima with a single phi measurement
            // using a beam spot projection. Should not be used in splashes or cosmics. 
            BooleanProperty m_recoverSinglePhiWithBS{this, "recoverSinglePhiHitsWithBS", true};
            // Flag to steer whether space points shall be downweighted according to their instance
            // multiplicity of the phi measurement such that it effectively contributes with weight 1
            BooleanProperty m_downWeightMultiplePrd{this, "downWeightPrdMultiplicity", false};
    };
}


#endif
