/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H
#define ACTSGEOMETRYINTERFACES_GEOMETRYDEFS_H

/// Load ATLAS Eigen library with custom geometry functions
#include "GeoPrimitives/GeoPrimitives.h"
/// Then load the Acts TypeDef definitions for Eigen
#include <string>
#include <string_view>
#include <ostream>
#ifndef SIMULATIONBASE 
#   include "ActsInterop/UnitConverters.h"
#   include "Acts/Utilities/OstreamFormatter.hpp"
#endif

#define ENUM_ITEM_STR(item) \
    case item: return #item;


namespace ActsTrk {
    /// Simple enum to Identify the Type of the
    /// ACTS sub detector
    enum class DetectorType: std::uint8_t {
        /// Inner detector legacy
        Pixel,
        Sct,
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
    /** @brief Define an enumeration to retrieve the
               envelope tracking volume from  */
    enum class SystemEnvelope: std::uint8_t {
        ITkExit,  // Envelope volume of the ITk
        CaloExit, // Enevelope volume of the Calorimeter / Ms entrance
        MsExit // Envelope around the Muon system
    };
    /** @brief Define the volume parts of the GeometryIdentifier for each ATLAS 
     *         subsystem centrally. The Ids are used in the Gen-3 Acts::TackingGeometry
     *         construction to assign GeometryIdentifiers to each volume in the system
     *         tree. */
    namespace detail::GeoVolIds{
        /** Volume Ids used within the ITk */
        constexpr std::size_t s_stripVolumeId = 20;
        constexpr std::size_t s_innerPixelVolumeId = 5;
        constexpr std::size_t s_outerPixelVolumeId = 10;
        constexpr std::size_t s_beamPipeVolumeId = 1;
        /** HGTD volume IDs */
        constexpr std::size_t s_hgtdPosVolumeId = 30;
        constexpr std::size_t s_hgtdNegVolumeId = 31;
        /** Volume Ids ofthe Calorimeter */
        constexpr std::size_t s_caloEnvelopeID = 39;
        constexpr std::size_t s_caloBarrelId = 40;
        /* Volume Ids used within the muon system */
        constexpr std::size_t s_muonBarrelId = 80;
        constexpr std::size_t s_muonEndcapAId = 81;
        constexpr std::size_t s_muonEndcapCId = 82;
        constexpr std::size_t s_muonEndcapMiddleAId = 83;
        constexpr std::size_t s_muonEndcapMiddleCId = 84;
    }
    namespace detail{
        /** @brief */
        inline std::string toString(const DetectorType type) {
            switch (type) {
                using enum DetectorType;
                ENUM_ITEM_STR(Pixel);
                ENUM_ITEM_STR(Sct);
                ENUM_ITEM_STR(Trt);
                ENUM_ITEM_STR(Hgtd);
                ENUM_ITEM_STR(Mdt);
                ENUM_ITEM_STR(Rpc);
                ENUM_ITEM_STR(Tgc);
                ENUM_ITEM_STR(Csc);
                ENUM_ITEM_STR(Mm);
                ENUM_ITEM_STR(sTgc);
                ENUM_ITEM_STR(UnDefined);
            }
            return "Unknown";
        }
        inline std::string toString(const SystemEnvelope type) {
            switch (type) {
                using enum SystemEnvelope;
                ENUM_ITEM_STR(ITkExit);
                ENUM_ITEM_STR(CaloExit);
                ENUM_ITEM_STR(MsExit);
            }
            return "Unknown";
        }
    }
    /** @brief Pipe the detector type to an outstream object */
    inline std::ostream& operator<<(std::ostream& ostr, const DetectorType type) {
        return (ostr<<detail::toString(type));
    }
    /** @brief Pipe the Volume envelope to an oustream object  */
    inline std::ostream& operator<<(std::ostream& ostr, const SystemEnvelope type) {
        return (ostr<<detail::toString(type));
    }

}  // namespace ActsTrk

#ifndef SIMULATIONBASE 
ACTS_OSTREAM_FORMATTER(ActsTrk::DetectorType);
ACTS_OSTREAM_FORMATTER(ActsTrk::SystemEnvelope);
#endif

#undef ENUM_ITEM_STR
#endif
