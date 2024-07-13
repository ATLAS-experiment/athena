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
#include "xAODMuonPrepData/RpcMeasurementContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "xAODMuonPrepData/sTgcMeasContainer.h"


namespace MuonR4{
    class MuonSpacePointMakerAlg: public AthReentrantAlgorithm {
        public:
            MuonSpacePointMakerAlg(const std::string& name, ISvcLocator* pSvcLocator);

            ~MuonSpacePointMakerAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
            StatusCode finalize() override;
        
        private:
            /// Helper struct to collect all space points per chamber
            struct spacePointsPerChamber{
                std::vector<MuonSpacePoint> etaHits{};
                std::vector<MuonSpacePoint> phiHits{};                
            };
            
            using PreSortedSpacePointMap = std::unordered_map<const MuonGMR4::MuonChamber*, spacePointsPerChamber>;
            using SpacePointBucketVec = std::vector<MuonSpacePointBucket>;
            // Helper class to quantify how many space points are made within a certain detector region
            class SpacePointStatistics{
                public:
                    SpacePointStatistics(const Muon::IMuonIdHelperSvc* idHelperSvc);
                
                    void addToStat(const std::vector<MuonSpacePoint>& spacePoints);
                    void dumpStatisics(MsgStream& msg) const;
                private:
                    struct StatField{
                        unsigned int measEtaPhi{0};
                        unsigned int measEta{0};
                        unsigned int measPhi{0};

                        unsigned int allHits() const;
                    };
                    // Field key
                    struct FieldKey{
                        using StIdx_t = Muon::MuonStationIndex::StIndex;
                        using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex; 
                        StIdx_t stIdx{StIdx_t::StUnknown};
                        TechIdx_t techIdx{TechIdx_t::TechnologyUnknown};
                        int eta{0};
                        bool operator<(const FieldKey& other) const;
                    };

                    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
                    std::mutex m_mutex{};
                    using StatMap_t = std::map<FieldKey, StatField>;
                    StatMap_t m_map{};
            };
  
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
            
            SG::ReadHandleKey<xAOD::RpcMeasurementContainer> m_rpcKey{this, "RpcKey", "xRpcMeasurements",
                                                                "Key to the uncalibrated 1D rpc hits"};
            
            SG::ReadHandleKey<xAOD::TgcStripContainer> m_tgcKey{this, "TgcKey", "xTgcStrips",
                                                                "Key to the uncalibrated 1D tgc hits"};

            SG::ReadHandleKey<xAOD::MMClusterContainer> m_mmKey{this, "MmKey", "xAODMMClusters",
                                                                "Key to the uncalibrated 1D Mm hits"};

            SG::ReadHandleKey<xAOD::sTgcMeasContainer> m_stgcKey{this, "sTgcKey", "xAODsTgcMeasurements"};


            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            SG::WriteHandleKey<MuonSpacePointContainer> m_writeKey{this, "WriteKey", "MuonSpacePoints"};

            Gaudi::Property<double> m_spacePointWindow{this, "spacePointWindowSize", 2.*Gaudi::Units::m,
                                                       "Maximal size of a space point bucket"};
            
            Gaudi::Property<double> m_spacePointOverlap{this, "spacePointOverlap", 25.*Gaudi::Units::cm,
                                                        "Hits that are within <spacePointOverlap> of the bucket margin. "
                                                        "Are copied to the next bucket"};
    
            Gaudi::Property<bool> m_doStat{this, "doStats", true, 
                                           "If enabled the algorithm keeps track how many hits have been made" };
            
            Gaudi::Property<unsigned int> m_capacityBucket{this,"CapacityBucket" , 50};
            std::unique_ptr<SpacePointStatistics> m_statCounter ATLAS_THREAD_SAFE{};
    };
}


#endif