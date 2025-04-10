/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_ActsVolumeIdToDetectorElementCollectionMap_H
#define ACTSTRK_ActsVolumeIdToDetectorElementCollectionMap_H
#include <vector>
#include <array>
#include <stdexcept>
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"


namespace ActsTrk {
   class ActsVolumeIdToDetectorElementCollectionMap {
   public:
      ActsVolumeIdToDetectorElementCollectionMap() { m_collections.push_back(nullptr); }
      void registerCollection( unsigned int volume_id, const InDetDD::SiDetectorElementCollection *collection) {
         std::vector<const InDetDD::SiDetectorElementCollection *>::const_iterator iter=std::find(m_collections.begin(), m_collections.end(), collection);
         unsigned char col_i;
         if (iter == m_collections.end()) {
            assert(m_collections.size()<256u);
            col_i=static_cast<unsigned char>(m_collections.size());
            m_collections.push_back(collection);
         }
         else {
            assert( iter - m_collections.begin()< 256u);
            col_i = static_cast<unsigned char>(iter - m_collections.begin());
         }
         if (m_collectionId.at(volume_id) != 0 && m_collectionId.at(volume_id) != col_i) {
            throw std::runtime_error("Volume id maps to multiple detector element collections.");
         }
         m_collectionId.at(volume_id) = col_i;
      }
      const InDetDD::SiDetectorElementCollection *collection(unsigned int volume_id) const {
         return m_collections.at( volume_id >= m_collectionId.size() ? static_cast<unsigned char>(0u) : m_collectionId[volume_id] );
      }
      const std::array<unsigned char,256>  &collecionMap() const { return m_collectionId; }
      const std::vector<const InDetDD::SiDetectorElementCollection*> &collections() const { return m_collections; }

   private:
      std::array<unsigned char,256>              m_collectionId{};
      std::vector<const InDetDD::SiDetectorElementCollection*> m_collections;
   };
}


#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CLASS_DEF( ActsTrk::ActsVolumeIdToDetectorElementCollectionMap,1230701610 , 1 )
CONDCONT_MIXED_DEF(ActsTrk::ActsVolumeIdToDetectorElementCollectionMap, 1289614908);

#endif
