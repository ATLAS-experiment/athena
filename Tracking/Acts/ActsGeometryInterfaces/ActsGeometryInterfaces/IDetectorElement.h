/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_IACTSDETECTORELEMENT_H
#define ACTSGEOMETRYINTERFACES_IACTSDETECTORELEMENT_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// In AthSimulation, the Acts core library is not available yet
#ifndef SIMULATIONBASE 
#   include "Acts/Geometry/DetectorElementBase.hpp"
#endif
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "Identifier/Identifier.h"

/** @brief ATLAS extension of the Acts::DetectorElementBase. The extension provides extra methods 
 *         to identify the element within the ATLAS identifier scheme and also the enum indicating to which
 *         tracking subsystem the DetectorElement belongs to. Finally, the detector element provides the 
 *         interface to optionally precache the aligned transformations in the external AlignmentStore of the geometry
 *         context. 
 */
namespace ActsTrk {
    class IDetectorElement
#ifndef SIMULATIONBASE    
     : public Acts::DetectorElementBase
#endif 
    {
    public:
        virtual ~IDetectorElement() = default;

        /// Returns the ATLAS identifier
        virtual Identifier identify() const = 0;
        /// Returns the detector element type
        virtual DetectorType detectorType() const = 0;
        /// Caches the aligned transformation in the provided store. Returns the number of cached elements
        virtual unsigned int storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const = 0;
    };
}  // namespace ActsTrk

#endif