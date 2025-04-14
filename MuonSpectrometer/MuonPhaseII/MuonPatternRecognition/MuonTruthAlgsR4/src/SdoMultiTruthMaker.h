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
    /** @brief The SdoMuliTruthMaker translates the HepMCParticle links associated with the SDO 
     *         into a PRD_MultiTruthCollection. The latter is the ingredient to match the xAOD::TruthParticle
     *         with the SDO hits in the MS. */
    class SdoMultiTruthMaker : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief IdHelperSvc for Identifier printing / manipulation */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Input key of the SDO container to translate (E.g. MDT_SDO) */
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitKey{this, "SimContainer", ""};
            /** @brief Output key of the written PRD_MultiTruthCollection */
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_writeKey{this, "WriteKey", ""};
    };
}
#endif