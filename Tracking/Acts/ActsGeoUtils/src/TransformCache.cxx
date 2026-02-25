
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/TransformCache.h>
#include <GeoModelKernel/GeoVDetectorElement.h>

namespace ActsTrk {
    TransformCacheBase::~TransformCacheBase() {
        TicketCounter::giveBackTicket(m_type, m_clientNo);
    }
    TransformCacheBase::TransformCacheBase(const IdentifierHash& hash,
                                           const DetectorType type): 
          m_hash{hash}, m_type{type} {}

    void TransformCacheBase::releaseNominalCache() const {
        std::unique_lock guard{m_mutex};
        m_nomCache.release();
    }
    DetectorType TransformCacheBase::detectorType() const { return m_type; }

    void TransformCache::releaseNominalCache() const {
        TransformCacheBase::releaseNominalCache();
        const GeoVDetectorElement* vParent = dynamic_cast<const GeoVDetectorElement*>(parent());
        if (vParent) {
            vParent->getMaterialGeom()->clearPositionInfo();
        }
    } 
}
