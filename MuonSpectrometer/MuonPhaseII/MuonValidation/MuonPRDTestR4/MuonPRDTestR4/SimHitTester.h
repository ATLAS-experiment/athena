/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_SIMHITEST_H
#define PRDTESTERR4_SIMHITEST_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"
namespace MuonValR4{
    class SimHitTester: public TesterModuleBase {
        public:
            SimHitTester(MuonTesterTree& tree,
                         const std::string& inContainer,
                         const ActsTrk::DetectorType detType,
                         MSG::Level msgLvl = MSG::Level::INFO);

            bool declare_keys() override final;

            bool fill(const EventContext& ctx) override final;

            unsigned push_back(const xAOD::MuonSimHit& hit);
            
        private:
           unsigned int fillHit(const EventContext& ctx,
                                const xAOD::MuonSimHit& hit);
           /** @brief Key of the SimHit container in the StoreGate */
           SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_key{};
           /** @brief Name of the SimHit collection in the output tree */
           std::string m_collName{};
           /** @brief Global position of the SimHits */
           ThreeVectorBranch m_globPos{parent(), m_collName+"_globPos"};
           /** @brief Global direction of the traversing particle generating the hit */
           ThreeVectorBranch m_globDir{parent(), m_collName+"_globDir"};
           /** @brief Local position of the produced simHit */           
           ThreeVectorBranch m_locPos{parent(), m_collName+"_localPos"};
           /** @brief Local direction of the traversing particle generating the hit */
           ThreeVectorBranch m_locDir{parent(), m_collName+"_localDir"};
           /** @brief Global time when the simHit was produced */
           VectorBranch<float>& m_globTime{parent().newVector<float>(m_collName+"_globalTime")};
           /** @brief Velocity of the simHit expressed in terms of beta */
           VectorBranch<float>& m_beta{parent().newVector<float>(m_collName+"_beta")};
           /** @brief PdgId of the particle generating the hit */
           VectorBranch<int>& m_pdgId{parent().newVector<int>(m_collName+"_pdgId")};
           /** @brief Energy deposited in the volume */
           VectorBranch<float>& m_energyDep{parent().newVector<float>(m_collName+"_energyDeposit")};
           /** @brief Kinetic energy of the traversing particle */
           VectorBranch<float>& m_kinE{parent().newVector<float>(m_collName+"_kineticEnergy")};
           /** @brief Mass of the traversing particle */
           VectorBranch<float>& m_mass{parent().newVector<float>(m_collName+"_mass")};
           /** @brief Pointer to the Identifier branch */
           std::shared_ptr<MuonIdentifierBranch> m_identifier{};
           /** @brief Identifier of the phi channel, if any **/
           VectorBranch<int>& m_phiChannel{parent().newVector<int>(m_collName+"_phiChannel")};

           PtEtaPhiEBranch m_truthParticleP4{parent(), m_collName+"_truthPart_"};
           CartesFourVecBranch m_truthParticleProdVtx{parent(), m_collName+"_prodVertex_", CartesFourVecBranch::Type::Pos};
           VectorBranch<int>& m_truthParticleBarcode{parent().newVector<int>(m_collName+"_truthPart_barcode")};
           
           /** @brief look up table of alrady dumped sim hits */
           std::unordered_map<const xAOD::MuonSimHit*, unsigned int> m_idxLookUp{};

    };
}
#endif
