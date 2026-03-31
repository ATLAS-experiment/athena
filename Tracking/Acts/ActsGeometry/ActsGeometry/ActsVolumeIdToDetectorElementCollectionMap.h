/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_ActsVolumeIdToDetectorElementCollectionMap_H
#define ACTSTRK_ActsVolumeIdToDetectorElementCollectionMap_H
#include <vector>
#include <array>
#include <stdexcept>
#include <algorithm>
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"


namespace ActsTrk {
   class ActsVolumeIdToDetectorElementCollectionMap {
   public:
      ActsVolumeIdToDetectorElementCollectionMap();
      void registerCollection( unsigned int volume_id, const InDetDD::SiDetectorElementCollection *collection);
      const InDetDD::SiDetectorElementCollection *collection(unsigned int volume_id) const {
         return m_collections.at( volume_id >= m_collectionId.size() ? static_cast<unsigned char>(0u) : m_collectionId[volume_id] );
      }
      const std::array<unsigned char,256>  &collecionMap() const { return m_collectionId; }
      const std::vector<const InDetDD::SiDetectorElementCollection*> &collections() const { return m_collections; }
      void registerDetectorType(unsigned int volume_id, unsigned int detector_type);
      const std::array<unsigned char,256>  &volumeIdToDetectorType() const { return m_detectorType; }
   private:
      std::array<unsigned char,256>              m_collectionId{};
      std::array<unsigned char,256>              m_detectorType{};
      std::vector<const InDetDD::SiDetectorElementCollection*> m_collections;
   };
}


#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CLASS_DEF( ActsTrk::ActsVolumeIdToDetectorElementCollectionMap,1230701610 , 1 )
CONDCONT_MIXED_DEF(ActsTrk::ActsVolumeIdToDetectorElementCollectionMap, 1289614908);

#endif
