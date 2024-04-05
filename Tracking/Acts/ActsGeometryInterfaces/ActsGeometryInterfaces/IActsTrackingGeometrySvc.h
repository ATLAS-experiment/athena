/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYSVC_H

#include "GaudiKernel/IService.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

#include <memory>


namespace Acts {
    class TrackingGeometry;
}

class IActsTrackingGeometrySvc : virtual public IService {
public:
    DeclareInterfaceID(IActsTrackingGeometrySvc, 1, 0);

    virtual ~IActsTrackingGeometrySvc() = default;
    /// Returns a pointer to the internal ACTS tracking geometry
    virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() = 0;
    /// Returns an empty nominal context without any alignment caches
    virtual const ActsGeometryContext& getNominalContext() const = 0;
    /// Loops through the volumes of the tracking geometry and caches the aligned transforms in the store
    virtual unsigned int populateAlignmentStore(ActsTrk::DetectorAlignStore& store) const = 0;
};

#endif
