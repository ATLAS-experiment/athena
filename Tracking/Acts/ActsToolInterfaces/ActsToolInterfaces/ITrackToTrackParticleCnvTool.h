/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ITRACKTOTRACKPARTICLECNVTOOL_H
#define ACTSTOOLINTERFACES_ITRACKTOTRACKPARTICLECNVTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "ActsEvent/TrackContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"

namespace ActsTrk {

  /** @brief Interface for a tool that converts a single Acts track proxy into an xAOD::TrackParticle.
   *
   *  The tool fills defining parameters, covariance, fit quality, hit summaries,
   *  expected layer patterns, and track state parameters/covariances on the
   *  provided TrackParticle object.
   */
  class ITrackToTrackParticleCnvTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(ITrackToTrackParticleCnvTool, 1, 0);

    /** @brief Convert a single Acts track proxy into an xAOD::TrackParticle.
     *  @param trackParticle the output TrackParticle to populate (must already be registered in a container)
     *  @param ctx the current Athena EventContext
     *  @param track the track proxy to convert
     *  @param perigeeSurface if non-null, extrapolate defining parameters to this surface;
     *         if null, use the track's reference surface as-is (DontRecalculate mode)
     *  @param beamspotData if non-null, decorate beam tilt information on the particle
     */
    virtual StatusCode convert(
      xAOD::TrackParticle& trackParticle,
      const EventContext& ctx,
      const ActsTrk::TrackContainer::ConstTrackProxy& track,
      const Acts::PerigeeSurface* perigeeSurface = nullptr,
      const InDet::BeamSpotData* beamspotData = nullptr) const = 0;
  };

}

#endif
