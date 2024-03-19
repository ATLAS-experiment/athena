
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/TransformCache.h>

#define THROW_RUNTIME(message)                         \
    {                                                  \
      std::stringstream except{};                      \
      except<<__FILE__<<":"<<__LINE__<<" ";            \
      except<<message<<std::endl;                      \
      throw std::runtime_error(except.str());          \
    }
namespace ActsTrk {
    TransformCache::TransformCache(const IdentifierHash& hash,
                                   TransformMaker maker,
                                   const IDetectorElement* parentEle): 
          m_hash{hash},
          m_transform{maker},
          m_parent{parentEle} {}

    const IDetectorElement* TransformCache::parent() const{
        return m_parent;
    }
    void TransformCache::releaseNominalCache() const {
        m_nomCache.release();
    } 
    const Amg::Transform3D& TransformCache::getTransform(const ActsTrk::AlignmentStore* alignStore) const {    
        /// Valid alignment store is given -> Take the transformation from the cache there
        if (alignStore){
            const Amg::Transform3D* cache = alignStore->getTransform(this);
            if (cache) return *cache;
            THROW_RUNTIME("The Alignment store does not contain an aligned transformation.");
        }
        /// Fall back solution to go onto the nominal cache    
        if (!m_nomCache) {
            return (*m_nomCache.set(std::make_unique<Amg::Transform3D>(m_transform(nullptr, m_hash))));
        }
        return (*m_nomCache);
    }

    void TransformCache::storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const {
        if (!alignStore.trackingAlignment) return;
        if (alignStore.trackingAlignment->getTransform(this)) {
            THROW_RUNTIME("Transformation has already been cached. Being called twice");
        }
        alignStore.trackingAlignment->setTransform(this, m_transform(&alignStore, m_hash));
        /// If an external alignment is given, the nominal cache can be released
        m_nomCache.release();
    }
    IdentifierHash TransformCache::hash() const { return m_hash; }
    const TransformCache::TransformMaker& TransformCache::transformMaker() const { return m_transform; }
}
#undef THROW_RUNTIME