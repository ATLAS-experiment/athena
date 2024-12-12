/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDTEST_RPCCABLINGTESTALG_H
#define MUONCONDTEST_RPCCABLINGTESTALG_H
/**
   Algorithm to test the validity of the RPC cabling
*/
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace Muon {
    class RpcCablingTestAlg : public AthAlgorithm {
        public:
            using AthAlgorithm::AthAlgorithm;
            virtual ~RpcCablingTestAlg() = default;
            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            virtual unsigned int cardinality() const override final { return 1; }
        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            // MuonDetectorManager from the conditions store
            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                    "Key of input MuonDetectorManager condition data"};

            SG::ReadCondHandleKey<RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap", "Key of input MDT cabling map"};

            Gaudi::Property<std::vector<std::string>> m_considStat{this, "TestStations", {"BIS"}, "Cabling only for stations from these stations are tested"};

            std::set<int> m_cabStat{};
    };
}
#endif