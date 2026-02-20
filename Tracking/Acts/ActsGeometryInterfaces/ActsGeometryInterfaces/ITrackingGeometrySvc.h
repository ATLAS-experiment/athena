/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H
#define ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H

#include "GaudiKernel/IService.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include <memory>


namespace Acts {
    class TrackingGeometry;
}

namespace ActsTrk{
class ITrackingGeometrySvc : virtual public IService {
public:
    DeclareInterfaceID(ActsTrk::ITrackingGeometrySvc, 1, 0);

    virtual ~ITrackingGeometrySvc() = default;
    /// Returns a pointer to the internal ACTS tracking geometry
    virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() = 0;
    /// Returns an empty nominal context without any alignment caches
    virtual const GeometryContext& getNominalContext() const = 0;
    /// Loops through the volumes of the tracking geometry and caches the aligned transforms in the store
    virtual unsigned int populateAlignmentStore(DetectorAlignStore& store) const = 0;
};
}

#endif
