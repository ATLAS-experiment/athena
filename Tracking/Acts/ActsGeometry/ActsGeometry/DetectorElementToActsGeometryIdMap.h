/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_DETECTORELEMENTTOACTSGEOMETRYIDMAP_H
#define ACTSTRK_DETECTORELEMENTTOACTSGEOMETRYIDMAP_H
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include <unordered_map>

namespace Acts {
   class Surface;
}

namespace ActsTrk {
   using DetectorElementKey=unsigned int;
   constexpr unsigned int DETELEMENT_TYPE_SHIFT = 28;
   constexpr unsigned int DETELEMENT_HASH_MASK = ~(1u<<31|1u<<30|1u<<29|1u<<28);
   inline
   DetectorElementKey makeDetectorElementKey(xAOD::UncalibMeasType meas_type, unsigned int identifier_hash) {
      assert( sizeof(xAOD::UncalibMeasType) <= sizeof(std::size_t) );
      assert( static_cast<std::size_t>( Acts::toUnderlying(meas_type)&((~DETELEMENT_HASH_MASK)>>DETELEMENT_TYPE_SHIFT)) == static_cast<std::size_t>(meas_type));
      assert( (identifier_hash & DETELEMENT_HASH_MASK) == identifier_hash);
      return (Acts::toUnderlying(meas_type) << DETELEMENT_TYPE_SHIFT) | (identifier_hash & DETELEMENT_HASH_MASK);
   }

   /** @brief Geometry identifier and surface of a detector element. */
   struct DetectorElementGeoInfo {
      Acts::GeometryIdentifier geoId{};
      const Acts::Surface* surface{nullptr};
   };

   struct DetectorElementToActsGeometryIdMap : std::unordered_map<ActsTrk::DetectorElementKey,
                                                                  DetectorElementGeoInfo>
   {
      // utilities to abstract what is actually stored
      static DetectorElementGeoInfo makeValue(const Acts::GeometryIdentifier &geo_id,
                                              const Acts::Surface *surface = nullptr) {
         return DetectorElementGeoInfo{geo_id, surface};
      }
      static const Acts::GeometryIdentifier &getValue(const value_type &element) {
         return element.second.geoId;
      }
      /** @brief Surface of the detector element, or nullptr if none was stored. */
      static const Acts::Surface *getSurface(const value_type &element) {
         return element.second.surface;
      }
   };
}

#endif
