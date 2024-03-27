/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H
#define MUONSPACEPOINTFORMATION_MUONSPACEPOINTMAKERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"


#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonSpacePoint/MuonSpacePointContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"


namespace MuonR4{
    class MuonSpacePointMakerAlg: public AthReentrantAlgorithm {
        public:
            MuonSpacePointMakerAlg(const std::string& name, ISvcLocator* pSvcLocator);

            ~MuonSpacePointMakerAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        
        private:
            /// Helper struct to collect all space points per chamber
            struct spacePointsPerChamber{
                std::vector<MuonSpacePoint> etaHits{};
                std::vector<MuonSpacePoint> phiHits{};                
            };
            
            using ChamberSorter = MuonGMR4::MuonDetectorManager::ChamberSorter;     
            using PreSortedSpacePointMap = std::map<const MuonGMR4::MuonChamber*, spacePointsPerChamber, ChamberSorter>;
            using SpacePointBucketVec = std::vector<MuonSpacePointBucket>;

            template <class ContType> StatusCode loadContainerAndSort(const EventContext& ctx,
                                                                      const SG::ReadHandleKey<ContType>& key,
                                                                      PreSortedSpacePointMap& fillContainer) const;
                                                    
            void distributePointsAndStore(const EventContext& ctx,
                                          spacePointsPerChamber&& hitsPerChamber,
                                          MuonSpacePointContainer& finalContainer) const;

            void distributePointsAndStore(const EventContext& ctx,
                                          std::vector<MuonSpacePoint>&& spacePoints,
                                          SpacePointBucketVec& splittedContainer) const;


            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtKey{this, "MdtKey", "xAODMdtCircles",
                                                                      "Key to the uncalibrated Drift circle measurements"};
            
            SG::ReadHandleKey<xAOD::RpcStripContainer> m_rpcKey{this, "RpcKey", "xRpcStrips",
                                                                "Key to the uncalibrated 1D rpc hits"};
            
            SG::ReadHandleKey<xAOD::TgcStripContainer> m_tgcKey{this, "TgcKey", "xTgcStrips",
                                                                "Key to the uncalibrated 1D tgc hits"};

            SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            SG::WriteHandleKey<MuonSpacePointContainer> m_writeKey{this, "WriteKey", "MuonSpacePoints"};
    };
}


#endif