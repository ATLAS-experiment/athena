/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONDETECTORDEFS_H
#define MUONGEOMODELR4_MUONDETECTORDEFS_H

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
///
#include <ActsGeometryInterfaces/GeometryContext.h>

#include <CxxUtils/ArrayHelper.h>
#include <CxxUtils/StringUtils.h>


#include <Identifier/Identifier.h>
#include <Identifier/IdentifierHash.h>

#include <functional>

#ifndef SIMULATIONBASE
#   include "Acts/Geometry/VolumeBounds.hpp"
#   include "Acts/Surfaces/Surface.hpp"
#endif

namespace Acts{
    class VolumeBounds;
    class Surface;
    class TrackingVolume;
}

//// This header contains common helper utilities and definitions
namespace MuonGMR4 {
    /** @brief Returns the half-X length @ negative Y for the parsed volume bounds (Trapezoid/ Cuboid) */
    double halfXlowY(const Acts::VolumeBounds& bounds);
    /** @brief Returns the half-Y length @ posiive Y for the parsed volume bounds (Trapezoid/ Cuboid) */
    double halfXhighY(const Acts::VolumeBounds& bounds);
    /** @brief Returns the half-Y length for the parsed volume bounds (Trapezoid/ Cuboid) */
    double halfY(const Acts::VolumeBounds& bounds);
    /** @brief Returns the half-Z length for the parsed volume bounds (Trapezoid/ Cuboid) */
    double halfZ(const Acts::VolumeBounds& bounds);
    /** @brief Copy the alignment deltas from the inStore to a new alignment store
     *  @param inStore: Alignment store from which the delta transforms are copied */
    std::unique_ptr<ActsTrk::DetectorAlignStore> copyDeltas(const ActsTrk::DetectorAlignStore& inStore);
    namespace detail {
        /** @brief Returns the rotation matrix from the readout element coordinate system
          *         into the AMDB coordinate system */
        Amg::Transform3D rotationToAMDB(const ActsTrk::DetectorType type);
    }

    /** @brief Returns whether the parsed type is muon
     *  @param type: The DetectorType enum */
    bool isMuon(const ActsTrk::DetectorType type);

    /** @brief Returns the boundary surface parallel to the x-y plane at negative local z
     *  @param volume: Reference to the tracking volume from which the surface is retrieved */
    const Acts::Surface* bottomBoundary(const Acts::TrackingVolume& volume);
    /** @brief Returns the boundary surface parallel to the x-y plane at positive local z
     *  @param volume: Reference to the tracking volume from which the surface is retrieved */
    const Acts::Surface* topBoundary(const Acts::TrackingVolume& volume);

}  // namespace MuonGMR4



#endif
