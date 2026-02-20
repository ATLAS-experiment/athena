/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_SIMHITTOTRUTHPARTASSOCALG_H
#define MUONTRUTHALGSR4_SIMHITTOTRUTHPARTASSOCALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    /** @brief This algorihm associates the SDO / sim hit Identifiers to the
     *          truth particle using the HepMCParticle uniqueID to match both.
     *          The Identifiers are decorated per category to the truth particle */
    class SimHitToTruthPartAssocAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief IdHelperSvc for Identifier printing / manipulation */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Data dependency on the truth input container to decorate */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
            /** @brief Decoration to the hit identifier vector */
            SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_hitDecorKey{this, "HitIdDecoration", m_truthKey, "hitIds"};
            /** @brief Data dependency on the sim hit container */
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitKey{this, "SimHitContainer", ""};
    }; 
}

#endif