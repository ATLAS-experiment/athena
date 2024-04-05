
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/TransformCache.h>
#include <GeoModelKernel/GeoVDetectorElement.h>

namespace ActsTrk {
    TransformCache::~TransformCache() {
        TicketCounter::giveBackTicket(m_type, m_clientNo);
    }
    TransformCache::TransformCache(const IdentifierHash& hash,
                                   TransformMaker maker,
                                   const IDetectorElement* parentEle): 
        TransformCache(hash,std::make_unique<TransformMaker>(maker), parentEle) {}
    TransformCache::TransformCache(const IdentifierHash& hash,
                                   std::shared_ptr<const TransformMaker> maker,
                                   const IDetectorElement* parentEle): 
          m_parent{parentEle},
          m_hash{hash},
          m_transform{maker}{}

    const IDetectorElement* TransformCache::parent() const {
        return m_parent;
    }
    void TransformCache::releaseNominalCache() const {
        std::unique_lock guard{m_mutex};
        m_nomCache.release();
        const GeoVDetectorElement* vParent =dynamic_cast<const GeoVDetectorElement*>(m_parent);
        if (vParent) vParent->getMaterialGeom()->clearPositionInfo();
    } 
    IdentifierHash TransformCache::hash() const { return m_hash; }
    const TransformCache::TransformMaker& TransformCache::transformMaker() const { return *m_transform; }
}
