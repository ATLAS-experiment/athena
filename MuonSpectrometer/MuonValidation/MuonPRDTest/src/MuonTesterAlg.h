/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPRDTEST_MUONTESTERALG_H
#define MUONPRDTEST_MUONTESTERALG_H
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IParticleFourMomBranch.h"
#include "xAODMuon/MuonContainer.h"

namespace MuonVal{
    class MuonTesterAlg : public AthHistogramAlgorithm {
        public:

            using AthHistogramAlgorithm::AthHistogramAlgorithm;

            virtual StatusCode initialize() override;
            virtual StatusCode finalize() override;
            virtual StatusCode execute(const EventContext& ctx) override;
            virtual unsigned int cardinality() const override final { return 1; }
        private:
            SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons"};

            MuonVal::MuonTesterTree m_tree{"BasicMuonTest", "MUONVALIDSTREAM"};

            std::shared_ptr<IParticleFourMomBranch> m_muonBr{};


    }; 
}

#endif