/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsToTrkConvertorAlg.h"

namespace ActsTrk {
  StatusCode ActsToTrkConvertorAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");
    ATH_CHECK(m_tracksContainerKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode ActsToTrkConvertorAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    // I/O
    ATH_MSG_DEBUG("Retrieving input track collection '" << m_tracksContainerKey.key() << "' ...");
    const ActsTrk::TrackContainer *inputTracks{nullptr};
    ATH_CHECK(SG::get(inputTracks, m_tracksContainerKey, ctx));
    auto trackCollection = m_ATLASConverterTool->convertActsToTrkContainer(ctx, *inputTracks);
    SG::WriteHandle outputTrackHandle{m_tracksKey, ctx};
    ATH_MSG_DEBUG("Output Tracks Collection `" << m_tracksKey.key() << "` created ...");
    ATH_CHECK(outputTrackHandle.record(std::move(trackCollection)));

    return StatusCode::SUCCESS;
  } 
}
