/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H
#define ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H

#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"

namespace Acts {
    class TrackingGeometry;
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
        /** @brief Retrieve the geometry context with alignment constants from store gate
         *  @param ctx: eventCotnext to access store gate */
        virtual const ActsGeometryContext& getGeometryContext(const EventContext& ctx) const = 0;
        /** @brief Returns the refrence to the nominal ActsGeometryContext. The context is hold
            by the tracking geometry service and does not contain any alignable transforms  */
        virtual const ActsGeometryContext& getNominalGeometryContext() const = 0;
        /** @brief Returns the pointer to the identifier mapping between Acts::surface ID
         *         & IdentifierHash of the ITk surfaces */
        virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const= 0;

};
}
#endif
