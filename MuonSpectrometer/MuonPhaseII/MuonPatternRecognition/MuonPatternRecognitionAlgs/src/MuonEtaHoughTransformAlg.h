/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__HOUGHTRANSFORMALG__H
#define MUONR4__HOUGHTRANSFORMALG__H

#include "MuonPatternEvent/StationHoughMaxContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include <xAODMuonPrepData/RpcStripContainer.h>
#include <xAODMuonPrepData/TgcStripContainer.h>
#include <MuonSpacePoint/MuonSpacePointContainer.h>
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "Gaudi/Property.h"

// muon includes


namespace MuonR4{
    
    /// @brief Algorithm to handle the eta hough transform 
    /// 
    /// This algorithm is responsible for the initial eta hough
    /// transform from space-points created in an upstream alg. 
    /// It will write HoughMaxima into the event store
    /// for downstream use. 
    class MuonEtaHoughTransformAlg: public AthReentrantAlgorithm{
        public:
                MuonEtaHoughTransformAlg(const std::string& name, ISvcLocator* pSvcLocator);
                virtual ~MuonEtaHoughTransformAlg() = default;
                virtual StatusCode initialize() override;
                virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            
            using HoughSetupForBucket = MuonHoughEventData::HoughSetupForBucket;
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;

            /// @brief pre-processing method called once per event. 
            /// Populates the event data with the space points for each
            /// bucket and identifies the optimal search space in each bucket.
            /// @param data: event data object
            /// @param space point list from store gate 
            StatusCode preProcess(MuonHoughEventData & data, 
                                  const MuonSpacePointContainer & spacePoints ) const; 

            /// @brief prepare the accumulator and the peak finder once per event 
            /// @param data: event data object
            StatusCode prepareHoughPlane(MuonHoughEventData & data) const; 
            
            /// @brief process a bucket. 
            /// Performs the hough transform in the given bucket and adds the maxima
            /// to the event data. 
            /// @param data: event data object
            /// @param currentBucket: bucket to process
            StatusCode processBucket(MuonHoughEventData & data, 
                                     HoughSetupForBucket& currentBucket) const; 

            /// @brief fill the accumulator from a given space point. 
            /// @param data: event data object 
            /// @param SP: space point to fill from 
            void fillFromSpacePoint(MuonHoughEventData & data, 
                                    const MuonR4::HoughHitType & SP) const; 
            
            /// @brief extend a maximum with all compatible (pure) phi hits. 
            /// @param hitList: list of hits to extend 
            /// @param bucket: the bucket to take the phi hits from 
            void extendWithPhiHits(std::vector<HoughHitType> & hitList, HoughSetupForBucket& bucket) const ;  

            // target resolution in the angle
            DoubleProperty m_targetResoTanTheta{this, "ResolutionTargetTanTheta", 0.02};
            // target resolution in the y intercept
            DoubleProperty m_targetResoIntercept{this, "ResolutionTargetIntercept", 10.};
            // minimum search window half width, tan(theta) 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchTanTheta{this, "minSigmasSearchTanTheta", 3.};
            // minimum search window half width, intercept 
            // - in multiples of the target resolution
            DoubleProperty m_minSigmasSearchIntercept{this, "minSigmasSearchIntercept", 3.};
            // number of accumulator bins for the angle 
            IntegerProperty m_nBinsTanTheta{this, "nBinsTanTheta", 20};
            // number of accumulator bins for the intercept 
            IntegerProperty m_nBinsIntercept{this, "nBinsIntercept", 200};

            // input space points from SG
            SG::ReadHandleKey<MuonR4::MuonSpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
            
            // output maxima for downstram processing
            SG::WriteHandleKey<StationHoughMaxContainer> m_maxima{this, "StationHoughMaxContainer", "MuonHoughStationMaxima"};

            // ACTS geometry context
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    };
}


#endif
