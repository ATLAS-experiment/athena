/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_TrackingAlignVisitor_H
#define ACTSGEOMETRY_TrackingAlignVisitor_H

#include "GeoPrimitives/GeoPrimitives.h"
//
#include "Acts/Geometry/TrackingGeometryVisitor.hpp"

namespace ActsTrk{
    class DetectorAlignStore;
    class TrackingGeoAlignVisitor final: public Acts::TrackingGeometryVisitor {
        public:
            TrackingGeoAlignVisitor(DetectorAlignStore & store) noexcept;

            TrackingGeoAlignVisitor(const TrackingGeoAlignVisitor& other) noexcept = delete;

            TrackingGeoAlignVisitor(TrackingGeoAlignVisitor&& other) = delete;

            TrackingGeoAlignVisitor& operator=(const TrackingGeoAlignVisitor& other) = delete;
            TrackingGeoAlignVisitor& operator=(TrackingGeoAlignVisitor&& other) = delete;
      
            void visitVolume(const Acts::TrackingVolume& volume) override final;

            void visitSurface(const Acts::Surface& surface) override final;

            unsigned alignedObjects() const;
        private:
            DetectorAlignStore& m_store;
            unsigned m_aligned{};
    };
}


#endif