/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSlimmer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkTrackSlimmer/TrackSlimmer.h"
#include "AthContainers/ConstDataVector.h"

Trk::TrackSlimmer::TrackSlimmer(const std::string& name,
                                ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator) {}

Trk::TrackSlimmer::~TrackSlimmer() = default;

StatusCode
Trk::TrackSlimmer::initialize()
{
  if (m_slimTool.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve TrkTrackSlimmingTool tool "
                  << m_slimTool);
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_trackLocation.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
Trk::TrackSlimmer::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
Trk::TrackSlimmer::execute(const EventContext& ctx) const
{
  auto InputHandles = m_trackLocation.makeHandles(ctx);
  auto InputIt = InputHandles.begin();
  auto InputE = InputHandles.end();
  for (; InputIt != InputE; ++InputIt) {
    auto& trackLocation = *InputIt;
    if (trackLocation.isValid()) {
      // loop through tracks, slimming them as you go.
      TrackCollection::const_iterator it = trackLocation->begin();
      TrackCollection::const_iterator itEnd = trackLocation->end();
      for (; it != itEnd; ++it) {
        m_slimTool->slimConstTrack(**it);
      } // Loop over Trk::Track
    }   // is valid
  }     // loop over input collections

  return StatusCode::SUCCESS;
}
