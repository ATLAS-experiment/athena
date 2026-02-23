/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_RPCDigitVARIABLES_H
#define MuonPRDTEST_RPCDigitVARIABLES_H

#include <string>

#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"
namespace MuonPRDTest {
    class RpcDigitVariables : public PrdTesterModule {
    public:
        RpcDigitVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);

        ~RpcDigitVariables() = default;

        bool fill(const EventContext& ctx) override final;

        bool declare_keys() override final;

    private:
        SG::ReadHandleKey<RpcDigitContainer> m_key{};
        ScalarBranch<unsigned int>& m_RPC_nDigits{parent().newScalar<unsigned int>("N_Digits_RPC")};
        VectorBranch<float>& m_RPC_dig_time{parent().newVector<float>("Digits_RPC_time")};
        VectorBranch<float>& m_RPC_tot{parent().newVector<float>("Digits_RPC_timeOverThresh")};
        ThreeVectorBranch m_RPC_dig_globalPos{parent(), "Digits_RPC_globalPos"};
        TwoVectorBranch m_RPC_dig_localPos{parent(), "Digits_RPC_localPos"};
        RpcIdentifierBranch m_RPC_dig_id{parent(), "Digits_RPC"};	
    };
}  // namespace MuonPRDTest
#endif  // MuonPRDTEST_RPCDigitVARIABLES_H
