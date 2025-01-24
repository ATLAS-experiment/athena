/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JetTruthTagAlg.h"
#include "AsgDataHandles/ReadHandle.h"

namespace CP {
    StatusCode JetTruthTagAlg::initialize() {
        ANA_CHECK(m_jets.initialize(m_systematicsList));
        ANA_CHECK(m_systematicsList.initialize());
        ANA_CHECK(m_truthJets.initialize());
        if (!m_isHS.empty())
            m_decIsHS.emplace(m_isHS);
        if (!m_isPU.empty())
            m_decIsPU.emplace(m_isPU);

        return StatusCode::SUCCESS;
    }

    StatusCode JetTruthTagAlg::execute(const EventContext &ctx) const {
        auto truthJets = SG::makeHandle(m_truthJets, ctx);
        if (!truthJets.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve " << m_truthJets.key());
            return StatusCode::FAILURE;
        }
        for (const auto& sys : m_systematicsList.systematicsVector()) {
            const xAOD::JetContainer *jets{nullptr};
            ANA_CHECK(m_jets.retrieve(jets, sys));

            for (const xAOD::Jet *jet : *jets) {
                bool isHS = false;
                bool isPU = true;
                for (const xAOD::Jet *truthJet : *truthJets) {
                    float dr = jet->p4().DeltaR(truthJet->p4());
                    if (dr < m_hsMaxDR && truthJet->pt() > m_hsMinPt)
                        isHS = true;
                    if (dr < m_puMinDR && truthJet->pt() > m_puMinPt)
                        isPU = false;
                    if (isHS && !isPU)
                        break;
                }
                if (m_decIsHS)
                    (*m_decIsHS)(*jet) = isHS;
                if (m_decIsPU)
                    (*m_decIsPU)(*jet) = isPU;
            }
        }
        return StatusCode::SUCCESS;
    }
}
