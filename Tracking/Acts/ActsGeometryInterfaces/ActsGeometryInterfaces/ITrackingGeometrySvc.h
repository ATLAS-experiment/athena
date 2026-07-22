/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H
#define ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H

#include "GaudiKernel/IService.h"

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include <memory>


namespace Acts {
    class TrackingGeometry;
    class TrackingVolume;
}

namespace ActsTrk{
    struct DetectorElementToActsGeometryIdMap;
}

namespace ActsTrk{

/** @brief Interface class for the ATLAS service providing the 
           ActsTrackingGeometry. The tracking geometry is built at
           the initialization stage of the Athena job and owned
           by the tracking geometry service */
class ITrackingGeometrySvc : virtual public IService {
public:
    DeclareInterfaceID(ActsTrk::ITrackingGeometrySvc, 1, 0);

    virtual ~ITrackingGeometrySvc() = default;
    /// Returns a pointer to the internal ACTS tracking geometry
    virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const = 0;
    /// Returns an empty nominal context without any alignment caches
    virtual const GeometryContext& getNominalContext() const = 0;
    /// Loops through the volumes of the tracking geometry and caches the aligned transforms in the store
    virtual unsigned int populateAlignmentStore(DetectorAlignStore& store) const = 0;
    /** @brief Returns the envelope volume from the tracking geometry that's 
               containing all volumes of the subsystem  */
    virtual const Acts::TrackingVolume* getEnvelope(const SystemEnvelope envType) const = 0;

    /** @brief Returns the pointer to the identifier mapping between Acts::surface ID
         *         & IdentifierHash of the ITk surfaces */
    virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const= 0;
};
}

#endif
