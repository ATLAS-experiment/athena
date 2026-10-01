/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JetTruthTagAlg.h"
#include "AsgDataHandles/ReadHandle.h"

#include <utility>
#include <vector>

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
        // cache the truth-jet four-vectors, dropping those that can't affect either label
        std::vector<std::pair<xAOD::Jet::FourMom_t, double>> truthP4s;
        truthP4s.reserve(truthJets->size());
        for (const xAOD::Jet *truthJet : *truthJets) {
            const double pt = truthJet->pt();
            if (pt > m_hsMinPt || pt > m_puMinPt)
                truthP4s.emplace_back(truthJet->p4(), pt);
        }
        for (const auto& sys : m_systematicsList.systematicsVector()) {
            const xAOD::JetContainer *jets{nullptr};
            ANA_CHECK(m_jets.retrieve(jets, sys, ctx));

            for (const xAOD::Jet *jet : *jets) {
                bool isHS = false;
                bool isPU = true;
                const xAOD::Jet::FourMom_t jetP4 = jet->p4();
                for (const auto& [truthP4, truthPt] : truthP4s) {
                    float dr = jetP4.DeltaR(truthP4);
                    if (dr < m_hsMaxDR && truthPt > m_hsMinPt)
                        isHS = true;
                    if (dr < m_puMinDR && truthPt > m_puMinPt)
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
