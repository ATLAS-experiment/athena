/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_IDETECTORELEMENT_H
#define ACTSGEOMETRYINTERFACES_IDETECTORELEMENT_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"

#include "Identifier/Identifier.h"

namespace ActsTrk {
    /** @brief ATLAS interface of a detector element. Detector element are
     *         connected with the Acts::SurfacePlacementBase and are usually
     *         responsible to align each surface properly. Further, they can
     *         provide information about the number of readout channels, the
     *         channel pitch, the channel length etc. */
    class IDetectorElement {
        public:
            /** @brief Default destructor */
            ~IDetectorElement() = default;
            /** @brief Return the ATLAS identifier */
            virtual Identifier identify() const = 0;
            /** @brief Returns the detector element type */
            virtual DetectorType detectorType() const = 0;
            /** @brief Returns the reference to the aligned local to global transform from
             *         the DetectorAlignStore actually holding the transform of the Detector element
             *  @param store: Pointer to the alignment store. If not provided the nominal transform
             *                is returned */
            virtual const Amg::Transform3D& localToGlobalTransform(const DetectorAlignStore* store) const = 0;
            /** @brief Returns the aligned local to global transform from the passed geometry context
             *  @param gctx: Reference to the ATLAS geometry context */
            virtual const Amg::Transform3D& localToGlobalTransform(const GeometryContext& gctx) const = 0;
            /** @brief Caches the aligned transformation in the provided store. Returns the number of cached elements*/
            virtual unsigned storeAlignedTransforms(DetectorAlignStore& store) const = 0;
    };
}  // namespace ActsTrk

#endif