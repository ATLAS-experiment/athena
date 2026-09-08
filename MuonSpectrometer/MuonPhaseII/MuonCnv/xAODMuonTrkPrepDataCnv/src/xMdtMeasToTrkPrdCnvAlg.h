/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  
#ifndef XAODMUONTRKPREPDATACNV_xMDTTOPRDCNVALG_H
#define XAODMUONTRKPREPDATACNV_xMDTTOPRDCNVALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"


#include "MuonPrepRawData/MdtPrepData.h"
#include "MuonPrepRawData/MdtTwinPrepData.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"


#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace MuonR4{
    /** @brief Conversion algorithm to turn xAOD::mdtMeasurements into Trk::MdtPrepData */
    class xMdtMeasToTrkPrdCnvAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
  
            virtual ~xMdtMeasToTrkPrdCnvAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_readKey{this, "xAODKey", "xMdtMeasurements"};

            SG::WriteDecorHandleKey<xAOD::MdtDriftCircleContainer> m_linkKey{this, "PrdLinkKey", m_readKey, "mdtTrkPrdLink"};

            SG::WriteHandleKey<Muon::MdtPrepDataContainer>  m_writeKey{this, "WriteKey", "MDT_DriftCircles"};

            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

   
    };

}

#endif