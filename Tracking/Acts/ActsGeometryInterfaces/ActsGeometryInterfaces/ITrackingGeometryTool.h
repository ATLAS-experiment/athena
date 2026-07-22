/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H
#define ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H

#include "ActsGeometryInterfaces/GeometryContext.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"

namespace Acts {
    class TrackingGeometry;
    class TrackingVolume;
}

namespace ActsTrk{
    struct DetectorElementToActsGeometryIdMap;


/** @brief Geometry helper tool extending the Tracking geometry service by the data dependency to
 *         fetch the geometry context from StoreGate */
class ITrackingGeometryTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(ActsTrk::ITrackingGeometryTool, 1, 0);
        /** @brief Access to the built Acts tracking geometry  */
        virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const = 0;
        /** @brief Returns the refrence to the nominal GeometryContext. The context is hold
            by the tracking geometry service and does not contain any alignable transforms  */
        virtual const GeometryContext& getNominalGeometryContext() const = 0;
        /** @brief Returns the pointer to the identifier mapping between Acts::surface ID
         *         & IdentifierHash of the ITk surfaces */
        virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const= 0;
        /** @brief Returns the envelope volume from the tracking geometry that's 
                   containing all volumes of the subsystem  */
        virtual const Acts::TrackingVolume* getEnvelope(const SystemEnvelope envType) const = 0;

};
}
#endif
