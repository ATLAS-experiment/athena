/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_SEGMENTVARIABLES_H
#define MUONPRDTEST_SEGMENTVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"

#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonPRDTest{
    class SegmentVariables : public PrdTesterModule {
        public:
            SegmentVariables(MuonTesterTree& tree, 
                             const std::string& containerKey,
                             const std::string& outName,
                             MSG::Level msglvl);

            bool fill(const EventContext& ctx) override final;
            bool declare_keys() override final;
        
            unsigned int push_back(const xAOD::MuonSegment& segment);
        private:
            unsigned int fill(const xAOD::MuonSegment& segment);
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_key{};
            std::string m_name{};
           
           
            ThreeVectorBranch m_pos{parent(), m_name+"_pos"};
            ThreeVectorBranch m_dir{parent(), m_name+"_dir"};

            VectorBranch<char>& m_etaIdx{parent().newVector<char>(m_name+"_etaIndex")};
            VectorBranch<uint8_t>& m_sector{parent().newVector<uint8_t>(m_name+"_sector")};

            VectorBranch<uint8_t>& m_chamberIdx{parent().newVector<uint8_t>(m_name+"_chamberIdx")};
            VectorBranch<float>& m_chi2{parent().newVector<float>(m_name+"_chi2")};
            VectorBranch<unsigned int>& m_nDoF{parent().newVector<unsigned int>(m_name+"_nDoF")};

            VectorBranch<uint8_t>& m_nPrecHits{parent().newVector<uint8_t>(m_name+"_nPrecHits")};
            VectorBranch<uint8_t>& m_nTrigEtaLayers{parent().newVector<uint8_t>(m_name+"_nTrigEtaLayers")};
            VectorBranch<uint8_t>& m_nTrigPhiLayers{parent().newVector<uint8_t>(m_name+"_nTrigPhiLayers")};

            bool m_filterMode{false};
            std::unordered_map<const xAOD::MuonSegment*, unsigned int> m_idxLookUp{};

    };
}
#endif