/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTR4_RPCDigitVARIABLESR4_H
#define PRDTESTR4_RPCDigitVARIABLESR4_H

#include <string>

#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "MuonTesterTree/TwoVectorBranch.h"
namespace MuonValR4 {
    class RpcDigitVariablesR4 : public TesterModuleBase {
    public:
        RpcDigitVariablesR4(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);

        ~RpcDigitVariablesR4() = default;

        bool fill(const EventContext& ctx) override final;

        bool declare_keys() override final;

    private:
        SG::ReadHandleKey<RpcDigitContainer> m_key{};
        ScalarBranch<unsigned int>& m_RPC_nDigits{parent().newScalar<unsigned int>("N_Digits_RPC")};
        VectorBranch<float>& m_RPC_dig_time{parent().newVector<float>("Digits_RPC_time")};
	VectorBranch<float>& m_RPC_dig_timeDiff{parent().newVector<float>("Digits_RPC_timeDifference")};
        VectorBranch<float>& m_RPC_tot{parent().newVector<float>("Digits_RPC_timeOverThresh")};
        ThreeVectorBranch m_RPC_dig_globalPos{parent(), "Digits_RPC_globalPosCenter"};
	ThreeVectorBranch m_RPC_dig_localPos3D{parent(), "Digits_RPC_localPos3DCenter"};
	ThreeVectorBranch m_RPC_dig_globalPosFix{parent(), "Digits_RPC_globalPos"};
        ThreeVectorBranch m_RPC_dig_localPos3DFix{parent(), "Digits_RPC_localPos3D"};
        // TwoVectorBranch m_RPC_dig_localPos{parent(), "Digits_RPC_localPos"};
        RpcIdentifierBranch m_RPC_dig_id{parent(), "Digits_RPC"};	
	VectorBranch<int>& m_RPC_secIndex{parent().newVector<int>("Digits_RPC_sectorIndex")};
	VectorBranch<std::string>& m_RPC_stationName{parent().newVector<std::string>("Digits_RPC_stationName")};
	VectorBranch<int>& m_RPC_layerIndex{parent().newVector<int>("Digits_RPC_layerIndex")};
	// debug branches
	VectorBranch<float>& m_RPC_dig_stripHalfLength{parent().newVector<float>("Digits_RPC_BIHalfLength")};
	VectorBranch<int>& m_RPC_groupIndex0{parent().newVector<int>("Digits_RPC_groupIndex0")};
	VectorBranch<int>& m_RPC_groupIndex1{parent().newVector<int>("Digits_RPC_groupIndex1")};

    };
}  // namespace MuonValR4
#endif  // PRDTESTR4_RPCDigitVARIABLESR4_H
