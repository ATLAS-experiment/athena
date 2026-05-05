/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Local include(s):
#include "CalibratedMuonsProvider.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowCopy.h"

namespace CP {

    CalibratedMuonsProvider::CalibratedMuonsProvider(const std::string& name, ISvcLocator* svcLoc) : AthAlgorithm(name, svcLoc) {}

    StatusCode CalibratedMuonsProvider::initialize() {
        ATH_CHECK(m_eventInfo.initialize());
        ATH_CHECK(m_inputKey.initialize());
        ATH_CHECK(m_outputKey.initialize());
        ///
        m_ptDecorKeys.emplace_back(m_outputKey.key() + ".InnerDetectorPt");
        m_ptDecorKeys.emplace_back(m_outputKey.key() + ".MuonSpectrometerPt");
        ATH_CHECK(m_ptDecorKeys.initialize());
        ATH_CHECK(m_rndNumKey.initialize(m_useRndNumber));
        ATH_CHECK(m_tool.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode CalibratedMuonsProvider::execute() {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        SG::ReadHandle<xAOD::MuonContainer> muons{m_inputKey, ctx};
        if (!muons.isValid()) {
            ATH_MSG_FATAL("No muon container found");
            return StatusCode::FAILURE;
        }

        xAOD::ShallowCopyResult_t<xAOD::MuonContainer> output =
            xAOD::shallowCopy(*muons, ctx);

        if (!output.first || !output.second) {
            ATH_MSG_FATAL("Creation of shallow copy failed");
            return StatusCode::FAILURE;
        }

        if (!setOriginalObjectLink(*muons, *output.first)) {
            ATH_MSG_ERROR("Failed to add original object links to shallow copy of " << m_inputKey);
            return StatusCode::FAILURE;
        }
        for (xAOD::Muon* iParticle : *(output.first)) {
            ATH_MSG_DEBUG(" Old pt=" << iParticle->pt());
            if (m_tool->applyCorrection(*iParticle).code() == CorrectionCode::Error) return StatusCode::FAILURE;
            ATH_MSG_DEBUG(" New pt=" << iParticle->pt());
        }
        SG::WriteHandle<xAOD::MuonContainer> writeHandle{m_outputKey, ctx};
        ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

}  // namespace CP
