/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_ISURFACEPLACEMENT_H
#define ACTSGEOMETRYINTERFACES_ISURFACEPLACEMENT_H

#include "ActsGeometryInterfaces/IDetectorElement.h"

#include "Acts/Surfaces/SurfacePlacementBase.hpp"

namespace ActsTrk {
    /** @brief Extension of the interface of the @ref Acts::SurfacePlacementBase
     *         for ATLAS. The usecase are all surfaces that are connected with
     *         the ATLAS readout planes. Additional methods that return the
     *         ATLAS identifier of the associated Surface and that return the 
     *         ATLAS Detector type are added. Further, the pointer to the 
     *         upstream IDetectorElement, which is eventually responsible for 
     *          aligning the surface, is also available. */
    class ISurfacePlacement : public Acts::SurfacePlacementBase {
        public:
            virtual ~ISurfacePlacement() = default;
            /** @brief Return the ATLAS identifier of the surface */
            virtual Identifier identify() const = 0;
            /** @brief Returns the detector element type */
            virtual DetectorType detectorType() const = 0;
            /** @brief Returns the detector element upstream */
            virtual const IDetectorElement* detectorElement() const = 0;
            /** @brief Define all ISurfacePlacements to be sensitive */
            virtual bool isSensitive() const override final { return true; }
    };
}



#endif