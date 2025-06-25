/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_MDTPRDVARIABLES_H
#define MuonPRDTEST_MDTPRDVARIABLES_H

#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"

namespace MuonPRDTest{
    class MDTPRDVariables : public PrdTesterModule {
    public:
        MDTPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~MDTPRDVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
    
    private:
        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_MDT_nPRD{parent().newScalar<unsigned int>("N_PRD_MDT")};

        ThreeVectorBranch m_MDT_PRD_globalPos{parent(), "PRD_MDT_globalPos"};
        VectorBranch<double>& m_MDT_PRD_radius{parent().newVector<double>( "PRD_MDT_radius")};
        MdtIdentifierBranch m_MDT_PRD_id{parent(), "PRD_MDT"};
        VectorBranch<float>& m_MDT_PRD_error{parent().newVector<float>("PRD_MDT_error")};
        VectorBranch<int>& m_MDT_PRD_adc{parent().newVector<int>("PRD_MDT_adc")};
        VectorBranch<int>& m_MDT_PRD_tdc{parent().newVector<int>("PRD_MDT_tdc")};
        VectorBranch<int>& m_MDT_PRD_status{parent().newVector<int>("PRD_MDT_status")};
    };
};

#endif  // MuonPRDTEST_MDTPRDVARIABLES_H