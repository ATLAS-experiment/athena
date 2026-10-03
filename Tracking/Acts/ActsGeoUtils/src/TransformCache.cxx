
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/TransformCache.h>
#include <ActsGeoUtils/SurfacePlacement.h>

#include <GeoModelKernel/GeoVDetectorElement.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/Surface.hpp"
#endif

namespace ActsTrk {
    AlignableGeoPositioning::~AlignableGeoPositioning() {
        TicketCounter::giveBackTicket(m_type, m_clientNo);
    }
    AlignableGeoPositioning::AlignableGeoPositioning(const IdentifierHash& hash,
                                                     const DetectorType type): 
          m_hash{hash}, m_type{type} {}

    void AlignableGeoPositioning::releaseNominalCache() const {
        m_nomCache.release();
    }

    bool AlignableGeoPositioning::storeTransform(DetectorAlignStore& store) const {
        if (store.detType != detectorType() || 
            store.trackingAlignment->getTransform(m_clientNo) != nullptr){
            return false;
        }
        store.trackingAlignment->setTransform(m_clientNo, Amg::toIsometry3D(fetchTransform(&store)));
        return true;
    }
    DetectorType AlignableGeoPositioning::detectorType() const { return m_type; }

    void IReadoutSurfacePositioning::releaseNominalCache() const {
        AlignableGeoPositioning::releaseNominalCache();
        const GeoVDetectorElement* vParent = dynamic_cast<const GeoVDetectorElement*>(parent());
        if (vParent) {
            vParent->getMaterialGeom()->clearPositionInfo();
        }
    } 

    IReadoutSurfacePositioning::~IReadoutSurfacePositioning() = default;
#ifndef SIMULATIONBASE
    const SurfacePlacement* IReadoutSurfacePositioning::placement() const { return m_placement.get(); }
#endif

}
