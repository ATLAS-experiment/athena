/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_SEGMENTVARIABLES_H
#define MUONPRDTEST_SEGMENTVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonPRDTest{
    /** @brief Tester module to dump the segment container. For each event, the 
     *         module dumps all segments. */
    class SegmentVariables : public PrdTesterModule {
        public:
            SegmentVariables(MuonTesterTree& tree, 
                             const std::string& containerKey,
                             const std::string& outName,
                             MSG::Level msglvl);

            bool fill(const EventContext& ctx) override final;
            bool declare_keys() override final;
        
            /** @brief push back a particuar segment */
            unsigned int push_back(const xAOD::MuonSegment& segment);
            /** @brief  */
            bool addVariable(std::shared_ptr<IAuxElementDecorationBranch> br);
        private:
            unsigned int fill(const xAOD::MuonSegment& segment);

            /** @brief Store gate key of the segment container. */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_key{};
            /** @brief Name of the segment collection in the output collection */
            std::string m_name{};
            /** @brief Global position of the segment */
            ThreeVectorBranch m_pos{parent(), m_name+"_pos"};
            /** @brief Global direction of the segment */
            ThreeVectorBranch m_dir{parent(), m_name+"_dir"};
            /** @brief Station-eta index of the segment's precision hits */
            VectorBranch<char>& m_etaIdx{parent().newVector<char>(m_name+"_etaIndex")};
            /** @brief MS-sector of the segment's precision hits */
            VectorBranch<uint8_t>& m_sector{parent().newVector<uint8_t>(m_name+"_sector")};
            /** @brief MS station of the segment */
            VectorBranch<uint8_t>& m_chamberIdx{parent().newVector<uint8_t>(m_name+"_chamberIdx")};
            /** @brief chi 2 of the segment fit */
            VectorBranch<float>& m_chi2{parent().newVector<float>(m_name+"_chi2")};
            /** @brief number of degrees of freedom */
            VectorBranch<unsigned int>& m_nDoF{parent().newVector<unsigned int>(m_name+"_nDoF")};
            /** @brief Number of precision hits i.e. Mdt / sTGC (strip) / Mm hits */
            VectorBranch<uint8_t>& m_nPrecHits{parent().newVector<uint8_t>(m_name+"_nPrecHits")};
            /** @brief Number of trigger hits, i.e. Rpc / Tgc, meausuring the eta coordinate */
            VectorBranch<uint8_t>& m_nTrigEtaLayers{parent().newVector<uint8_t>(m_name+"_nTrigEtaLayers")};
            /** @brief Number of trigger hits, i.e. Rpc / Tgc, meausuring the phi coordinate */
            VectorBranch<uint8_t>& m_nTrigPhiLayers{parent().newVector<uint8_t>(m_name+"_nTrigPhiLayers")};
            /** @brief  */
            std::vector<std::shared_ptr<IAuxElementDecorationBranch>> m_addBranches{};
            bool m_filterMode{false};
            std::unordered_map<const xAOD::MuonSegment*, unsigned int> m_idxLookUp{};

    };
}
#endif