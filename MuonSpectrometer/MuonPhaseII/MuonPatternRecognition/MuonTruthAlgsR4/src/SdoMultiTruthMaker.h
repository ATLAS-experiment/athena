/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_SDOMULTITRUTHMAKER_H
#define MUONTRUTHALGSR4_SDOMULTITRUTHMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    class SdoMultiTruthMaker : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitKey{this, "SimContainer", ""};
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_writeKey{this, "WriteKey", ""};
    };
}
#endif