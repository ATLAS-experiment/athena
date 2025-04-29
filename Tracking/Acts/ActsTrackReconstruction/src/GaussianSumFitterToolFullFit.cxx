/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GaussianSumFitterTool.h"

// ATHENA
#include "TrkTrackSummary/TrackSummary.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "ActsEvent/TrackContainer.h"

// STL
#include <vector>

namespace ActsTrk {

std::unique_ptr<Trk::Track>
GaussianSumFitterTool::performFit(const EventContext& ctx,
			      const Acts::GeometryContext& tgContext,
			      const Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>& gsfOptions,
			      const std::vector<Acts::SourceLink>& trackSourceLinks,
			      const Acts::BoundTrackParameters& initialParams) const
{
  if(m_useDirectNavigation){
    ATH_MSG_ERROR("ACTS GSF UseDirectNavigation is true, but standard navigation is used");
    return nullptr;
  }
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }

  ActsTrk::MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
			      initialParams, gsfOptions, tracks);

  // Convert
  if (not result.ok()) return nullptr;
  return makeTrack(ctx, tgContext, tracks, result);
}

}

