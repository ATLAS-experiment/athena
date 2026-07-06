/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  
#ifndef XAODMUONTRKPREPDATACNV_xRPCTOPRDCNVALG_H
#define XAODMUONTRKPREPDATACNV_xRPCTOPRDCNVALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuonPrepData/RpcMeasurementContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace MuonR4{
    /** @brief Conversion algorithm to turn xAOD::RpcMeasurements into Trk::RpcPrepData */
    class xRpcMeasToTrkPrdCnvAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
  
            virtual ~xRpcMeasToTrkPrdCnvAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadHandleKey<xAOD::RpcMeasurementContainer> m_readKey{this, "ReadKey", "xRpcMeasurements"};

            SG::WriteDecorHandleKey<xAOD::RpcMeasurementContainer> m_linkKey{this, "PrdLinkKey", m_readKey, "rpcTrkPrdLink"};

            SG::WriteHandleKey<Muon::RpcPrepDataContainer>  m_writeKey{this, "WriteKey", "RPC_Measurements", "Key for RPC PRD Container"};

            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

   
    };

}

#endif