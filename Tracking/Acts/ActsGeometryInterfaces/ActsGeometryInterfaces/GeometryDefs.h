/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H
#define ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H

/// Load ATLAS Eigen library with custom geometry functions
#include "GeoPrimitives/GeoPrimitives.h"
/// Then load the Acts TypeDef definitions for Eigen
#include <string>
#ifndef SIMULATIONBASE 
#   include "ActsInterop/UnitConverters.h"
#endif
namespace ActsTrk {
    /// Simple enum to Identify the Type of the
    /// ACTS sub detector
    enum class DetectorType: unsigned short {
        /// Inner detector legacy
        Pixel,
        Sct,
        /// Maybe the Sct / Pixel for Itk become seperate entries?
        Trt,
        Hgtd,
        /// MuonSpectrometer
        Mdt,  /// Monitored Drift Tubes
        Rpc,  /// Resitive Plate Chambers
        Tgc,  /// Thin gap champers
        Csc,  /// Maybe not needed in the migration
        Mm,   /// Micromegas (NSW)
        sTgc,  /// Small Thing Gap chambers (NSW)
        UnDefined
    };

    inline std::string to_string(const DetectorType& type) {
        if (type == DetectorType::Pixel)
            return "Pixel";
        else if (type == DetectorType::Sct)
            return "Sct";
        else if (type == DetectorType::Trt)
            return "Trt";
        else if (type == DetectorType::Hgtd)
            return "Hgtd";
        else if (type == DetectorType::Mdt)
            return "Mdt";
        else if (type == DetectorType::Rpc)
            return "Rpc";
        else if (type == DetectorType::Tgc)
            return "Tgc";
        else if (type == DetectorType::Csc)
            return "Csc";
        else if (type == DetectorType::Mm)
            return "Mm";
        else if (type == DetectorType::sTgc)
            return "sTgc";
        return "Unknown";
    }

}  // namespace ActsTrk
#endif
