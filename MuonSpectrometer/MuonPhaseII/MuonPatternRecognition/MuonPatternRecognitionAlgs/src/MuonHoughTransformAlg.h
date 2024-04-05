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
#include "MuonHoughEventData.h"
#include "Gaudi/Property.h"

// muon includes


namespace MuonR4{
    
    class MuonHoughTransformAlg: public AthReentrantAlgorithm{
        public:
                MuonHoughTransformAlg(const std::string& name, ISvcLocator* pSvcLocator);
                virtual ~MuonHoughTransformAlg() = default;
                virtual StatusCode initialize() override;
                virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            
            using HoughSpaceInBucket = MuonHoughEventData::HoughSpaceInBucket;
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;

            
            StatusCode preProcess(MuonHoughEventData & data, 
                                  const MuonSpacePointContainer & driftCircles ) const; 
            
            StatusCode prepareHoughPlane(MuonHoughEventData & data) const; 
            
            StatusCode prepareStation(MuonHoughEventData & data, 
                                      const MuonGMR4::MuonChamber* station) const; 
            StatusCode processBucket(MuonHoughEventData & data, 
                                     HoughSpaceInBucket& currentBucket) const; 

            void fillFromSpacePoint(MuonHoughEventData & data, 
                                    const MuonR4::HoughHitType & SP) const; 

            DoubleProperty m_targetResoTanTheta{this, "ResolutionTargetTanTheta", 0.02};
            DoubleProperty m_targetResoZ0{this, "ResolutionTargetZ0", 10.};
            IntegerProperty m_nBinsTanTheta{this, "nBinsTanTheta", 10};
            IntegerProperty m_nBinsZ0{this, "nBinsZ0", 100};

            SG::ReadHandleKey<MuonR4::MuonSpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
            
            // TODO: Other technologies to join once their EDM exists

            SG::WriteHandleKey<StationHoughMaxContainer> m_maxima{this, "StationHoughMaxContainer", "MuonHoughStationMaxima"};

            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    };
}


#endif
