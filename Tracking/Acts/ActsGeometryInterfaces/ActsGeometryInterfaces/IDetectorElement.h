/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_IDETECTORELEMENT_H
#define ACTSGEOMETRYINTERFACES_IDETECTORELEMENT_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// In AthSimulation, the Acts core library is not available yet
#ifndef SIMULATIONBASE 
#   include "Acts/Surfaces/SurfacePlacementBase.hpp"
#endif
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "Identifier/Identifier.h"

/** @brief ATLAS extension of the Acts::SurfacePlacementBase. The extension provides extra methods 
 *         to identify the element within the ATLAS identifier scheme and also the enum indicating to which
 *         tracking subsystem the DetectorElement belongs to. Finally, the detector element provides the 
 *         interface to optionally precache the aligned transformations in the external AlignmentStore of the geometry
 *         context. 
 */
namespace ActsTrk {
    /** @brief base class interface providing the bare minimal interface
     *         extension. Each associated detector element returned by an Acts::Surface 
     *         is castable to an IDetectorElementBase.
     */ 
    class IDetectorElementBase
#ifndef SIMULATIONBASE    
     : public Acts::SurfacePlacementBase
#endif 
    {
        public:
            /** @brief Default destructor */
            ~IDetectorElementBase() = default;
            /** @brief Return the ATLAS identifier */
            virtual Identifier identify() const = 0;
            /** @brief Returns the detector element type */
            virtual DetectorType detectorType() const = 0;
#ifndef SIMULATIONBASE    
            using Acts::SurfacePlacementBase::localToGlobalTransform;
#endif
            /** @brief Returns the reference to the aligned local to global transform from
             *         the StoreObject actually holding the transform of the Detector element
             *  @param store: Pointer to the alignment store. If not provided the nominal transform
             *                is returned */
            virtual const Amg::Transform3D& localToGlobalTransform(const DetectorAlignStore* store) const = 0;
    };
    /** @brief Base class interface for the actual readout elements. */    
    class IDetectorElement : public IDetectorElementBase {
    public:
        /** @brief Default destructor */
        virtual ~IDetectorElement() = default;
        /** @brief Caches the aligned transformation in the provided store. Returns the number of cached elements*/
        virtual unsigned storeAlignedTransforms(DetectorAlignStore& store) const = 0;
    };
}  // namespace ActsTrk

#endif