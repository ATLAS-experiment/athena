/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingGeoAlignVisitor.h"
///
#include "ActsGeometryInterfaces/IVolumePlacement.h"
#include "ActsGeometryInterfaces/ISurfacePlacement.h"
//

#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Surfaces/Surface.hpp"

namespace ActsTrk{
    TrackingGeoAlignVisitor::TrackingGeoAlignVisitor(DetectorAlignStore& store) noexcept:
            m_store{store}{}

    void TrackingGeoAlignVisitor::visitVolume(const Acts::TrackingVolume& volume) {
        if (!volume.isAlignable()) {
            return;
        }
        const auto* placement = dynamic_cast<const IVolumePlacement*>(volume.volumePlacement());
        if(!placement) {
            return;
        }
        m_aligned += placement->storeAlignedTransforms(m_store);
    }

    void TrackingGeoAlignVisitor::visitSurface(const Acts::Surface& surface) {
        // Alignment of the portals is taking care by the VolumePlacement.
        if (!surface.isAlignable() || !surface.isSensitive()) {
            return;
        }

        const auto* placement = dynamic_cast<const ISurfacePlacement*>(surface.surfacePlacement());
        if(!placement) {
            return;
        }
        m_aligned += placement->detectorElement()->storeAlignedTransforms(m_store);
    }
    unsigned TrackingGeoAlignVisitor::alignedObjects() const { return m_aligned; }
}