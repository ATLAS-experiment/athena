// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "TrkTrackCollectionMerger/AddTrackSummaryAlg.h"

namespace Trk {

AddTrackSummaryAlg::AddTrackSummaryAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode AddTrackSummaryAlg::initialize() {
  ATH_MSG_DEBUG("Initializing " << name());

  // Check configuration
  ATH_CHECK(m_inputTrackCollection.initialize());
  ATH_CHECK(m_outputTrackCollection.initialize());
  ATH_CHECK(m_trackSummaryTool.retrieve());

  ATH_MSG_INFO("Will add TrackSummary to: " << m_inputTrackCollection.key());
  ATH_MSG_INFO("Output collection: " << m_outputTrackCollection.key());

  return StatusCode::SUCCESS;
}

StatusCode AddTrackSummaryAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name());

  // Retrieve the input track collection
  SG::ReadHandle<TrackCollection> inputTracks(m_inputTrackCollection, ctx);
  if (!inputTracks.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve track collection: " << m_inputTrackCollection.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Retrieved " << inputTracks->size() << " tracks from " << m_inputTrackCollection.key());

  // Create output collection - use VIEW to avoid copying track objects
  auto outputTracks = std::make_unique<ConstDataVector<TrackCollection>>(SG::VIEW_ELEMENTS);
  outputTracks->reserve(inputTracks->size());

  // Loop through tracks and add summary to those missing it
  int tracksProcessed = 0;
  int tracksAlreadyHaveSummary = 0;

  for (const Trk::Track* track : *inputTracks) {
    if (!track) {
      ATH_MSG_WARNING("Null track pointer in collection");
      continue;
    }

    // Check if track already has a summary
    if (!track->trackSummary()) {
      bool doSummary = true;

      const Trk::Perigee* per = track->perigeeParameters();
      if (per) {
        const float pt  = per->momentum().perp();
        const float eta = per->momentum().eta();

        if (m_minPt > 0.0 && pt < m_minPt) doSummary = false;
        if (m_maxAbsEta < 90.0 && std::abs(eta) > m_maxAbsEta) doSummary = false;
      } else {
        // No perigee: be conservative to avoid crashes later
        doSummary = true;
      }
      // Create and attach the summary (updateTrackSummary modifies track in-place)
      // This is thread-safe because each event gets its own copy of tracks
      if (doSummary){
      m_trackSummaryTool->updateTrackSummary(ctx, *const_cast<Trk::Track*>(track));
      ++tracksProcessed;

      // Verify the summary was actually created
      if (!track->trackSummary()) {
        ATH_MSG_ERROR("Failed to create TrackSummary for track!");
        return StatusCode::FAILURE;
      }
     }
    } else {
      ++tracksAlreadyHaveSummary;
    }

    // Add track pointer to output collection
    outputTracks->push_back(track);
  }

  // Record output collection
  SG::WriteHandle<ConstDataVector<TrackCollection>> outputHandle(m_outputTrackCollection, ctx);
  ATH_CHECK(outputHandle.record(std::move(outputTracks)));

  return StatusCode::SUCCESS;
}

} // namespace Trk

