/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_IVolumePlacement_H
#define ACTSGEOMETRYINTERFACES_IVolumePlacement_H
#ifndef SIMULATIONBASE 

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// In AthSimulation, the Acts core library is not available yet
#include "Acts/Geometry/VolumePlacementBase.hpp"

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"

namespace ActsTrk{
    /** @brief ATLAS extension of the VolumePlacementBase interface */
    class IVolumePlacement : public Acts::VolumePlacementBase {
        public:
            /** @brief Detector type of the store where the associated transforms are cached  */
            virtual DetectorType detectorType() const = 0;
            /** @brief Fill the alignment store wwith the transforms from the element & Return
             *         the number of cached transforms
             *  @param store: Reference to the store which will be populated. */
            virtual unsigned storeAlignedTransforms(DetectorAlignStore& store) const = 0;
    };
}
#endif
#endif