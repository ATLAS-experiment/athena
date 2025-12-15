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
#endif

namespace Acts{
    class VolumeBounds;
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
}  // namespace MuonGMR4

#endif
