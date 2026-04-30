/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  
#ifndef XAODMUONTRKPREPDATACNV_xTGCTOPRDCNVALG_H
#define XAODMUONTRKPREPDATACNV_xTGCTOPRDCNVALG_H



#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODMuonPrepData/TgcStripContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace MuonR4 {
    class xTgcMeasToTrkPrdCnvAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;

            virtual StatusCode execute(const EventContext& ctx) const override final;

        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadHandleKey<xAOD::TgcStripContainer> m_readKey{this, "ReadKey", "xTgcStrips"};

            SG::WriteHandleKey<Muon::TgcPrepDataContainer>  m_writeKey{this, "WriteKey", "TGC_MeasurementsAllBCs", "Key for RPC PRD Container"};

            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

    };
}

#endif