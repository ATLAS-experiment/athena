/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_TGCPRDVARIABLES_H
#define MuonPRDTEST_TGCPRDVARIABLES_H

#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"

namespace MuonPRDTest{
    class TGCPRDVariables : public PrdTesterModule {
    public:
        TGCPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~TGCPRDVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
    
    private:
        SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_TGC_nPRD{parent().newScalar<unsigned int>("N_PRD_TGC")};

        ThreeVectorBranch m_TGC_PRD_globalPos{parent(), "PRD_TGC_globalPos"};
        TwoVectorBranch m_TGC_PRD_localPos{parent(), "PRD_TGC_localPos"};
        TgcIdentifierBranch m_TGC_PRD_id{parent(), "PRD_TGC"};
        VectorBranch<uint8_t>& m_TGC_PRD_bcId{parent().newVector<uint8_t>("PRD_TGC_bcId")};
        VectorBranch<float>& m_TGC_PRD_error{parent().newVector<float>("PRD_TGC_error")};
    };
};

#endif  // MuonPRDTEST_TGCPRDVARIABLES_H