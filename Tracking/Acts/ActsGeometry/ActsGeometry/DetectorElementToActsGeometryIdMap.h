/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_DETECTORELEMENTTOACTSGEOMETRYIDMAP_H
#define ACTSTRK_DETECTORELEMENTTOACTSGEOMETRYIDMAP_H

#include "xAODMeasurementBase/MeasurementDefs.h"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include <unordered_map>

namespace ActsTrk {
   namespace {
      template <typename T_EnumClass >
      inline
      constexpr typename std::underlying_type<T_EnumClass>::type to_underlying(T_EnumClass an_enum) {
         return static_cast<typename std::underlying_type<T_EnumClass>::type>(an_enum);
      }
   }

   using DetectorElementKey=unsigned int;
   constexpr unsigned int DETELEMENT_TYPE_SHIFT = 28;
   constexpr unsigned int DETELEMENT_HASH_MASK = ~(1<<31|1<<30|1<<29|1<<28);
   inline
   DetectorElementKey makeDetectorElementKey(xAOD::UncalibMeasType meas_type, unsigned int identifier_hash) {
      assert( (to_underlying(meas_type)&((~DETELEMENT_HASH_MASK)>>DETELEMENT_TYPE_SHIFT)) == meas_type);
      assert( (identifier_hash & DETELEMENT_HASH_MASK) == identifier_hash);
      return (to_underlying(meas_type) << DETELEMENT_TYPE_SHIFT) | (identifier_hash & DETELEMENT_HASH_MASK);
   }
   struct DetectorElementToActsGeometryIdMap : std::unordered_map<ActsTrk::DetectorElementKey,
                                                                  Acts::GeometryIdentifier>
   {
      // utilities to abstract what is actually stored
      static const Acts::GeometryIdentifier &makeValue(const Acts::GeometryIdentifier &geo_id) {
         return geo_id;
      }
      static const Acts::GeometryIdentifier &getValue(const value_type &element) {
         return element.second;
      }
   };
}


#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CLASS_DEF( ActsTrk::DetectorElementToActsGeometryIdMap, 98583818, 1 )
CONDCONT_DEF(ActsTrk::DetectorElementToActsGeometryIdMap, 14180);

#endif
