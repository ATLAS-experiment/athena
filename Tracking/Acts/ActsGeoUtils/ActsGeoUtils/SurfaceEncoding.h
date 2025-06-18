/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_SurfaceEncoding_h
#define ActsEvent_SurfaceEncoding_h

#ifndef SIMULATIONBASE

#include <xAODTracking/TrackSurface.h>
#include <xAODTracking/TrackSurfaceAuxContainer.h>

#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "Acts/Surfaces/Surface.hpp"

namespace ActsTrk {

/**
 * Prepares persistifiable representation of surface into xAOD::TrackSurface
 * object
 * @warning supports only few types, unhandled surface type results in a
 * exception
 * @arg backend - container to store surface data
 * @arg index - index under which the data needs to be recorded
 */

void encodeSurface(xAOD::TrackSurfaceAuxContainer* backend, size_t index,
                   const Acts::Surface* surface,
                   const Acts::GeometryContext& geoContext);
/**
* As above, but works on xAOD::TrackSurface object
*/

void encodeSurface(xAOD::TrackSurface* backend,
                   const Acts::Surface* surface,
                   const Acts::GeometryContext& geoContext);



/**
 * Creates transient Acts Surface objects given a surface backend
 * implementation should be exact mirror of encodeSurface
 */

std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::TrackSurface* backend, const Acts::GeometryContext& geoContext);

/**
* As above, but takes data from Aux container at an index i
*/
std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::TrackSurfaceAuxContainer* backend, size_t i, const Acts::GeometryContext& geoContext);


}  // namespace ActsTrk

#endif
#endif