/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IRefineTrackingGeoTool_H
#define ACTSGEOMETRYINTERFACES_IRefineTrackingGeoTool_H

#include "Acts/Geometry/TrackingGeometryVisitor.hpp"
#include "GaudiKernel/IAlgTool.h"

namespace ActsTrk{
    /** @brief Interface to pass mutable TrackingGeometry visitors to the 
     *         built tracking geometry before the geometry is becoming a
     *         const object.  */
    class IRefineTrackingGeoTool: virtual public IAlgTool, 
                                  public Acts::TrackingGeometryMutableVisitor {
        public:
           /** @brief Declare the interface ID */
           DeclareInterfaceID(ActsTrk::IRefineTrackingGeoTool, 1, 0);
           /** @brief Default desstructor */
           virtual ~IRefineTrackingGeoTool() = default;
    };
}

#endif