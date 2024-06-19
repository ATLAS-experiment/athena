/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

        private:
           SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_key{};

           std::string m_collName{};
           ThreeVectorBranch m_globPos{parent(), m_collName+"GlobPos"};
           ThreeVectorBranch m_globDir{parent(), m_collName+"GlobDir"};
           
           ThreeVectorBranch m_locPos{parent(), m_collName+"LocalPos"};
           ThreeVectorBranch m_locDir{parent(), m_collName+"LocalDir"};

           VectorBranch<float>& m_globTime{parent().newVector<float>(m_collName+"GlobalTime")};
           VectorBranch<float>& m_beta{parent().newVector<float>(m_collName+"Beta")};
           VectorBranch<int>& m_pdgId{parent().newVector<int>(m_collName+"PdgId")};
           VectorBranch<float>& m_energyDep{parent().newVector<float>(m_collName+"EnergyDeposit")};
           VectorBranch<float>& m_kinE{parent().newVector<float>(m_collName+"KinericEnergy")};
           VectorBranch<float>& m_mass{parent().newVector<float>(m_collName+"Mass")};
           std::shared_ptr<MuonIdentifierBranch> m_identifier{};
    };
}
#endif