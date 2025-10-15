/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNITIONALGS_ETAHOUGHTRANSFORMALG__H
#define MUONR4_MUONPATTERNRECOGNITIONALGS_ETAHOUGHTRANSFORMALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include "StoreGate/WriteHandleKey.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonPatternEvent/HoughEventData.h>
#include <MuonSpacePoint/SpacePointContainer.h>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// muon includes


namespace MuonR4{
    
    /// @brief Algorithm to handle the eta hough transform 
    /// 
    /// This algorithm is responsible for the initial eta hough
    /// transform from space-points created in an upstream alg. 
    /// It will write HoughMaxima into the event store
    /// for downstream use. 
    class EtaHoughTransformAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;    

            virtual ~EtaHoughTransformAlg() = default;
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            
            using HoughSetupForBucket = HoughEventData::HoughSetupForBucket;
            /// @brief pre-processing method called once per event. 
            /// Populates the event data with the space points for each
            /// bucket and identifies the optimal search space in each bucket.
            /// @param ctx: EventContext to parse to the visualization tool
            /// @param gctx: Geometry context to retrieve global positioning of the chambers
            /// @param spacePoints point list from store gate 
            /// @param data: event data object
            void preProcess(const EventContext& ctx,
                            const ActsGeometryContext& gctx,
                            const SpacePointContainer & spacePoints,
                            HoughEventData & data) const; 

            /// @brief prepare the accumulator and the peak finder once per event 
            /// @param data: event data object
            void prepareHoughPlane(HoughEventData & data) const; 
            
            /// @brief process a bucket. 
            /// Performs the hough transform in the given bucket and adds the maxima
            /// to the event data. 
            /// @param ctx: EventContext to parse to the visualization tool
            /// @param data: event data object
            /// @param currentBucket: bucket to process
            void processBucket(const EventContext& ctx,
                               HoughEventData & data, 
                               HoughSetupForBucket& currentBucket) const; 

            /// @brief fill the accumulator from a given space point. 
            /// @param data: event data object 
            /// @param SP: space point to fill from 
            void fillFromSpacePoint(HoughEventData & data, 
                                    const MuonR4::HoughHitType & SP) const; 

            /// @brief apply quality cuts on a given maximum 
            bool passSeedQuality (const HoughSetupForBucket& currentBucket, const MuonR4::ActsPeakFinderForMuon::Maximum & maximum) const; 
            
            /// @brief extend a maximum with all compatible (pure) phi hits. 
            /// @param hitList: list of hits to extend 
            /// @param bucket: the bucket to take the phi hits from 
            void extendWithPhiHits(std::vector<HoughHitType> & hitList, HoughSetupForBucket& bucket) const ;  
            /// @brief Returns whether the hit is a precision hit or not
            static bool isPrecisionHit(const HoughHitType& hit);
            // target resolution in the angle
            DoubleProperty m_targetResoTanTheta{this, "ResolutionTargetTanTheta", 0.05};
            // target resolution in the y intercept
            DoubleProperty m_targetResoIntercept{this, "ResolutionTargetIntercept", 10.};
            // minimum search window half width, tan(theta) 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchTanTheta{this, "minSigmasSearchTanTheta", 2.0};
            // minimum search window half width, intercept 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchIntercept{this, "minSigmasSearchIntercept", 2.0};
            // Cut on the number of weighted hits on the maximum
            DoubleProperty m_peakThreshold{this, "peakThreshold", 2.5};
            // Minimum distance in tanBeta between two maxima
            DoubleProperty m_minMaxDistTheta{this, "MaximumSeparationTheta", 0.};
            // Minimum distance in the intercept between two maxima
            DoubleProperty m_minMaxDistIntercept{this, "MaximumSeparationIntercept", 15.};
            // Fraction of weighted counts around the peak to be associated to the maximum
            DoubleProperty m_peakFractionCutOff{this, "PeakFractionCutOff", 0.6};
            // How many valid precision hits have to be on the pattern
            UnsignedIntegerProperty m_nPrecHitCut{this, "nMinPrecHits", 3};
            
            // number of accumulator bins for the angle 
            IntegerProperty m_nBinsTanTheta{this, "nBinsTanTheta", 7};
            // number of accumulator bins for the intercept 
            IntegerProperty m_nBinsIntercept{this, "nBinsIntercept", 15};
            // Flag to steer whether space points shall be downweighted according to their instance
            // multiplicity of the phi measurement such that it effectively contributes with weight 1
            BooleanProperty m_downWeightMultiplePrd{this, "downWeightPrdMultiplicity", false};
            /// Handle to the IdHelperSvc
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            // input space points from SG
            SG::ReadHandleKey<SpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
            // output maxima for downstram processing
            SG::WriteHandleKey<EtaHoughMaxContainer> m_maxima{this, "EtaHoughMaxContainer", "MuonHoughStationMaxima"};
            // ACTS geometry context
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /// Pattern visualization tool
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};

    };
}


#endif
