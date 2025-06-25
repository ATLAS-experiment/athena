/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_RPCPRDVARIABLES_H
#define MuonPRDTEST_RPCPRDVARIABLES_H

#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"

namespace MuonPRDTest{
    class RPCPRDVariables : public PrdTesterModule {
    public:
        RPCPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~RPCPRDVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
    
    private:
        SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_RPC_nPRD{parent().newScalar<unsigned int>("N_PRD_RPC")};
        ThreeVectorBranch m_RPC_PRD_globalPos{parent(), "PRD_RPC_globalPos"};
        VectorBranch<double>& m_RPC_PRD_locX{parent().newVector<double>( "PRD_RPC_locX")};
        RpcIdentifierBranch m_RPC_PRD_id{parent(), "PRD_RPC"};
        VectorBranch<float>& m_RPC_PRD_error{parent().newVector<float>("PRD_RPC_error")};
        VectorBranch<float>& m_RPC_PRD_time{parent().newVector<float>("PRD_RPC_time")};
    };
};

#endif  // MuonPRDTEST_RPCPRDVARIABLES_H