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
    /** @brief TrackingGeometryVisitor to load the aligned surface and volume
     *         transforms into the DetectorAlignStore of a given subdetector.
     *         The tracking geometry tree is traversed and the store is passed
     *         to the VolumePlacement and SurfacePlacement instances. If the 
     *         detectorType of the two matches, the placement writes its
     *         aligned transforms ino the store */
    class TrackingGeoAlignVisitor final: public Acts::TrackingGeometryVisitor {
        public:
            /** @brief Constructor
                @param store; Mutable reference to the alignment store to fill */
            explicit TrackingGeoAlignVisitor(DetectorAlignStore & store) noexcept;
            /** @brief Delete the copy constructor */
            TrackingGeoAlignVisitor(const TrackingGeoAlignVisitor& other) noexcept = delete;
            /** @brief Delete the move constructor */
            TrackingGeoAlignVisitor(TrackingGeoAlignVisitor&& other) = delete;
            /** @brief Delete the copy assignemnt operator */
            TrackingGeoAlignVisitor& operator=(const TrackingGeoAlignVisitor& other) = delete;
            /** @brief Delete the move assignemnt operator */
            TrackingGeoAlignVisitor& operator=(TrackingGeoAlignVisitor&& other) = delete;
            /** @brief Override the visitVolume to align the volume and its portals */
            void visitVolume(const Acts::TrackingVolume& volume) override final;
            /** @brief Override the visitSurface to align the sensitive surfaces */
            void visitSurface(const Acts::Surface& surface) override final;
            /** @brief Returns how many transforms have been written into the store */
            unsigned alignedObjects() const;
        private:
            DetectorAlignStore& m_store;
            unsigned m_aligned{};
    };
}
#endif