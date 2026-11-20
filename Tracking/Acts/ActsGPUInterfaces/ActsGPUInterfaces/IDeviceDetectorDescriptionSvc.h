/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUGEOMETRY_IDEVICEDETECTORDESCRIPTIONSVC_H
#define ACTSGPUGEOMETRY_IDEVICEDETECTORDESCRIPTIONSVC_H

#include "GaudiKernel/IService.h"
#include "Identifier/Identifier.h"

#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "detray/geometry/identifier.hpp"

#include <vector>

struct StaticCondEntry {
    unsigned int designId = 0;
    detray::geometry::identifier detrayGeometryId{};
    Acts::GeometryIdentifier::Value actsGeometryId = 0;
    Identifier athenaId;       // only meaningful if hasAthenaModule
    bool hasAthenaModule = false;
    bool isPixel = false;      // only meaningful if hasAthenaModule
};

namespace ActsTrk {

/**
 * @class IDeviceDetectorDescriptionSvc
 *
 * @brief Interface of the service providing the static device detector description
 *
 * Gives access to the per-module information, that does not change during
 * the run, in the order of the rows of the detector conditions description.
 */
class IDeviceDetectorDescriptionSvc : virtual public IService
{
public:

    DeclareInterfaceID(IDeviceDetectorDescriptionSvc, 1, 0);

    /// Static per-module entries, indexed like the detector conditions description
    virtual const std::vector<StaticCondEntry>& staticCondEntries() const = 0;

};

} // namespace ActsTrk

#endif // ACTSGPUGEOMETRY_IDEVICEDETECTORDESCRIPTIONSVC_H
