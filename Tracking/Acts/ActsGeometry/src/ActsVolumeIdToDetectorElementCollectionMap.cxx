#include  "ActsGeometry/ActsVolumeIdToDetectorElementCollectionMap.h"
#include <cassert>
#include <algorithm>

namespace ActsTrk {
   ActsVolumeIdToDetectorElementCollectionMap::ActsVolumeIdToDetectorElementCollectionMap() {
      m_collections.push_back(nullptr);
      std::fill(m_detectorType.begin(),m_detectorType.end(),static_cast<unsigned char>(255));
   }
   void ActsVolumeIdToDetectorElementCollectionMap::registerCollection( unsigned int volume_id, const InDetDD::SiDetectorElementCollection *collection) {
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
   void ActsVolumeIdToDetectorElementCollectionMap::registerDetectorType(unsigned int volume_id, unsigned int detector_type) {
      assert( volume_id < std::numeric_limts<unsigned char>::max() );
      assert( detector_type < 255 );
      if (m_detectorType[volume_id] == 255) {
         m_detectorType[volume_id]=detector_type;
      }
      else if (detector_type != m_detectorType[volume_id]) {
         throw std::runtime_error("The same volume id is registered to multiple detector types." );
      }
   }
}
