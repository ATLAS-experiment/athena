/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_IMATERIALWRITERTOOL_H
#define ACTSMATERIAL_IMATERIALWRITERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

namespace ActsTrk {
    /// @class IMaterialWriterTool
    ///
    /// Interface definition for material writing
    class IMaterialWriterTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(IMaterialWriterTool, 1, 0);
        virtual ~IMaterialWriterTool() = default;
        virtual void writeMaterial(const ActsTrk::GeometryContext& gctx,
                                   const Acts::TrackingGeometryMaterial& detMaterial) const = 0;
    };

}

#endif
