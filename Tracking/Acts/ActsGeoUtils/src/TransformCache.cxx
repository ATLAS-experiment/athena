
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
                                   const DetectorType type): 
          m_hash{hash},
          m_type{type} {}

    void TransformCache::releaseNominalCache() const {
        std::unique_lock guard{m_mutex};
        m_nomCache.release();
        const GeoVDetectorElement* vParent = dynamic_cast<const GeoVDetectorElement*>(parent());
        if (vParent) vParent->getMaterialGeom()->clearPositionInfo();
    } 
    DetectorType TransformCache::detectorType() const { return m_type; }
}
