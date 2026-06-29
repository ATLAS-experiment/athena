/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaussianSumFitterTool.h"

// ATHENA
#include "TrkTrackSummary/TrackSummary.h"

// ACTS
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "ActsEvent/TrackContainer.h"

// STL
#include <vector>

namespace ActsTrk {

std::unique_ptr<Trk::Track>
GaussianSumFitterTool::performDirectFit(const EventContext& ctx,
			      const Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>& gsfOptions,
			      const std::vector<Acts::SourceLink>& trackSourceLinks,
			      const Acts::BoundTrackParameters& initialParams,
			      const std::vector<const Acts::Surface*>& surfaces) const
{
  if (!m_useDirectNavigation) {
    ATH_MSG_ERROR("ACTS GSF UseDirectNavigation is false, but direct navigation is used");
    return nullptr;
  }
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }


  ActsTrk::MutableTrackBackend trackContainerBackEnd;
  ActsTrk::MutableTrackStateBackend multiTrajBackEnd;
  ActsTrk::MutableTrackContainer tracks( std::move(trackContainerBackEnd),
                                         std::move(multiTrajBackEnd));
  auto result = m_directFitter->fit(trackSourceLinks.begin(),
			      trackSourceLinks.end(),
			      initialParams,
			      gsfOptions,
			      surfaces,
			      tracks);

  return m_ATLASConverterTool->convertFitResult(ctx, result,
                                                Trk::TrackInfo::TrackFitter::GaussianSumFilter);
}

}

