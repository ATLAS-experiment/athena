/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/JvtSelectionToolBase.h"
#include "AsgDataHandles/ReadDecorHandle.h"

namespace CP {
    StatusCode JvtSelectionToolBase::initialize() {
        m_etaAcc = SG::ConstAccessor<float>(m_jetEtaName);
        m_cutPos = m_info.addCut("Jvt", "Whether the jet passes the Jvt selection");

        return StatusCode::SUCCESS;
    }

    const asg::AcceptInfo &JvtSelectionToolBase::getAcceptInfo() const { return m_info; }

    asg::AcceptData JvtSelectionToolBase::accept(const xAOD::IParticle *jet) const {
        asg::AcceptData data(&m_info);
        if (!isInRange(jet)) {
            data.setCutResult(m_cutPos, true);
            return data;
        }
        data.setCutResult(m_cutPos, select(jet));
        return data;
    }

    bool JvtSelectionToolBase::isInRange(const xAOD::IParticle *jet) const {
        if (jet->pt() < m_minPtForJvt || jet->pt() > m_maxPtForJvt)
            return false;
        float eta = m_etaAcc(*jet);
        return std::abs(eta) >= m_minEta && std::abs(eta) <= m_maxEta;
    }
} // namespace CP
