/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H
#define ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H

/// Load ATLAS Eigen library with custom geometry functions
#include "GeoPrimitives/GeoPrimitives.h"
/// Then load the Acts TypeDef definitions for Eigen
#include <string>
#include <ostream>
#ifndef SIMULATIONBASE 
#   include "ActsInterop/UnitConverters.h"
#   include "Acts/Utilities/OstreamFormatter.hpp"
#endif
namespace ActsTrk {
    /// Simple enum to Identify the Type of the
    /// ACTS sub detector
    enum class DetectorType: std::uint8_t {
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
        switch (type) {
            using enum DetectorType;
            case Pixel: return "Pixel";
            case Sct: return "Sct";
            case Trt: return "Trt";
            case Hgtd: return "Hgtd";
            case Mdt: return "Mdt";
            case Rpc: return "Rpc";
            case Tgc: return "Tgc";
            case Mm: return "Mm";
            case sTgc: return "sTgc";
            case Csc: return "Csc";
            case UnDefined: return "UnDefined";
        }
        return "Unknown";
    }

    inline std::ostream& operator<<(std::ostream& ostr, const DetectorType type) {
        return (ostr<<to_string(type));
    }

}  // namespace ActsTrk

#ifndef SIMULATIONBASE 
ACTS_OSTREAM_FORMATTER(ActsTrk::DetectorType);
#endif
#endif
