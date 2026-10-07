/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_IMATERIALDUMPERTOOL_H
#define ACTSMATERIAL_IMATERIALDUMPERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

namespace Acts {
    class TrackingGeometry;
}

namespace ActsTrk {
    /// @class IMaterialDumperTool
    ///
    /// Dumps tracking geometry material through the configured material writers.
    /// Used both at the end of the material mapping, with the mapped material,
    /// and by the material dumper algorithm, with the material already on the geometry.
    class IMaterialDumperTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(IMaterialDumperTool, 1, 0);
        virtual ~IMaterialDumperTool() = default;
        /// Dump a material map, e.g. the output of the material mapping
        virtual void dumpMaterial(const ActsTrk::GeometryContext& gctx,
                                  const Acts::TrackingGeometryMaterial& material) const = 0;
        /// Collect the material assigned to the surfaces and volumes of a geometry and dump it
        virtual void dumpGeometryMaterial(const ActsTrk::GeometryContext& gctx,
                                          const Acts::TrackingGeometry& geometry) const = 0;
    };

}

#endif
