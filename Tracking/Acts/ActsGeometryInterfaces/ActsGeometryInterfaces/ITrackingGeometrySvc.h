/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H
#define ACTSGEOMETRYINTERFACES_ITrackingGeometrySvc_H

#include "GaudiKernel/IService.h"

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include <memory>

#ifdef ACTSGEOMETRY_HAVE_DETRAY
#include <detray/core/detector.hpp>
#include <detray/detectors/itk_metadata.hpp>
#endif


namespace Acts {
    class TrackingGeometry;
    class TrackingVolume;
}

namespace ActsTrk{
    struct DetectorElementToActsGeometryIdMap;
}

namespace ActsTrk{
#ifdef ACTSGEOMETRY_HAVE_DETRAY
/// @brief Detray metadata used when converting the Acts::TrackingGeometry into
///        a Detray geometry. detray::itk_metadata is generated for the ATLAS ITk
using DetrayMetadata = detray::itk_metadata<detray::array<float>>;
using DetrayDetector = detray::detector<DetrayMetadata>;
#endif

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

#ifdef ACTSGEOMETRY_HAVE_DETRAY
    /** @brief Returns the Detray geometry converted from the Acts::TrackingGeometry.
               Only populated when the service was configured to build it.
               Only declared in builds where ACTS was compiled with the Detray
               plugin (Acts::PluginDetray). */
    virtual std::shared_ptr<const ActsTrk::DetrayDetector> detrayGeometry() const = 0;
#endif
};
}

#endif
