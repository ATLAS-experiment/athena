/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_MMDigitVARIABLES_H
#define MuonPRDTEST_MMDigitVARIABLES_H

#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"

namespace MuonPRDTest{
    class MMDigitVariables : public PrdTesterModule {
    public:
        MMDigitVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~MMDigitVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
    
    private:
        SG::ReadHandleKey<MmDigitContainer> m_key{};
        ScalarBranch<unsigned int>& m_NSWMM_nDigits{parent().newScalar<unsigned int>("N_Digits_MM")};
        VectorBranch<float>& m_NSWMM_dig_time{parent().newVector<float>("Digits_MM_time")};
        VectorBranch<float>& m_NSWMM_dig_charge{parent().newVector<float>("Digits_MM_charge")};
        TwoVectorBranch m_NSWMM_dig_stripLpos{parent(), "Digits_MM_stripLpos"};        
        ThreeVectorBranch m_NSWMM_dig_stripGpos{parent(), "Digits_MM_stripGpos"};       
        MmIdentifierBranch m_NSWMM_dig_id{parent(), "Digits_MM"};
    };
};
#endif  // MuonPRDTEST_MMDigitVARIABLES_H