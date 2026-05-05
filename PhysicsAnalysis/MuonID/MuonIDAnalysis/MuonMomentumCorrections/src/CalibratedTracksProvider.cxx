/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "CalibratedTracksProvider.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowCopy.h"

namespace CP {

    CalibratedTracksProvider::CalibratedTracksProvider(const std::string& name, ISvcLocator* svcLoc) : AthAlgorithm(name, svcLoc) {}

    StatusCode CalibratedTracksProvider::initialize() {
        ATH_CHECK(m_eventInfo.initialize());
        ATH_CHECK(m_inputKey.initialize());
        ATH_CHECK(m_outputKey.initialize());
        ATH_CHECK(m_rndNumKey.initialize(m_useRndNumber));
        ATH_CHECK(m_tool.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode CalibratedTracksProvider::execute() {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_inputKey, ctx};
        if (!tracks.isValid()) {
            ATH_MSG_FATAL("No muon container found");
            return StatusCode::FAILURE;
        }

        xAOD::ShallowCopyResult_t<xAOD::TrackParticleContainer> output =
            xAOD::shallowCopy(*tracks, ctx);

        if (!output.first || !output.second) {
            ATH_MSG_FATAL("Creation of shallow copy failed");
            return StatusCode::FAILURE;
        }

        if (!setOriginalObjectLink(*tracks, *output.first)) {
            ATH_MSG_ERROR("Failed to add original object links to shallow copy of " << m_inputKey);
            return StatusCode::FAILURE;
        }

        for (xAOD::TrackParticle* iParticle : *(output.first)) {
            ATH_MSG_VERBOSE("Old pt=" << iParticle->pt());
            if (m_tool->applyCorrectionTrkOnly(*iParticle, m_detType).code() == CorrectionCode::Error) return StatusCode::FAILURE;
            ATH_MSG_VERBOSE("New pt=" << iParticle->pt());
        }
        SG::WriteHandle<xAOD::TrackParticleContainer> writeHandle{m_outputKey, ctx};
        ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));

        // Return gracefully:
        return StatusCode::SUCCESS;
    }

}  // namespace CP
