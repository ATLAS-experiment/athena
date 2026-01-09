/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GaudiKernel/SystemOfUnits.h>

#ifndef SIMULATIONBASE
#   include <Acts/Geometry/VolumeBounds.hpp>
#   include <Acts/Geometry/CuboidVolumeBounds.hpp>
#   include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#endif

namespace MuonGMR4 {
    std::unique_ptr<ActsTrk::DetectorAlignStore> copyDeltas(const ActsTrk::DetectorAlignStore& inStore) {
        auto newStore = std::make_unique<ActsTrk::DetectorAlignStore>(inStore);
        if(newStore->geoModelAlignment) {
            if (!inStore.geoModelAlignment->posCacheLocked()) {
                newStore->geoModelAlignment->clearPosCache();
            }
            newStore->trackingAlignment = std::make_unique<ActsTrk::DetectorAlignStore::TrackingAlignStore>(inStore.detType);
        }
        return newStore;
    }
    namespace detail{
        Amg::Transform3D rotationToAMDB(const ActsTrk::DetectorType type) {
            return type == ActsTrk::DetectorType::sTgc ? Amg::Transform3D::Identity()
                  : Amg::getRotateY3D(90. * Gaudi::Units::deg) * Amg::getRotateZ3D(90. * Gaudi::Units::deg);
        }
    }

#ifndef SIMULATIONBASE
    double halfXlowY(const Acts::VolumeBounds& visitBounds) {
        switch (visitBounds.type()) {
            case Acts::VolumeBounds::BoundsType::eCuboid: {
                const auto& bounds = static_cast<const Acts::CuboidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::CuboidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthX);
            }
            case Acts::VolumeBounds::BoundsType::eTrapezoid: {
                const auto& bounds = static_cast<const Acts::TrapezoidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthXnegY);
            } default:
                THROW_EXCEPTION("Unsupported bound type "<<visitBounds.type());
        }
        return 0.;
    }
    double halfXhighY(const Acts::VolumeBounds& visitBounds) {
        switch (visitBounds.type()) {
            case Acts::VolumeBounds::BoundsType::eCuboid: {
                const auto& bounds = static_cast<const Acts::CuboidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::CuboidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthX);
            }
            case Acts::VolumeBounds::BoundsType::eTrapezoid: {
                const auto& bounds = static_cast<const Acts::TrapezoidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthXposY);
            } default:
                THROW_EXCEPTION("Unsupported bound type "<<visitBounds.type());
        }
        return 0.;
    }
    double halfY(const Acts::VolumeBounds& visitBounds) {
        switch (visitBounds.type()) {
            case Acts::VolumeBounds::BoundsType::eCuboid: {
                const auto& bounds = static_cast<const Acts::CuboidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::CuboidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthY);
            }
            case Acts::VolumeBounds::BoundsType::eTrapezoid: {
                const auto& bounds = static_cast<const Acts::TrapezoidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthY);
            } default:
                THROW_EXCEPTION("Unsupported bound type "<<visitBounds.type());
        }
        return 0.;

    }
    double halfZ(const Acts::VolumeBounds& visitBounds) {
        switch (visitBounds.type()) {
            case Acts::VolumeBounds::BoundsType::eCuboid: {
                const auto& bounds = static_cast<const Acts::CuboidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::CuboidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthZ);
            }
            case Acts::VolumeBounds::BoundsType::eTrapezoid: {
                const auto& bounds = static_cast<const Acts::TrapezoidVolumeBounds&>(visitBounds);
                using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;
                return bounds.get(BoundEnum::eHalfLengthZ);
            } default:
                THROW_EXCEPTION("Unsupported bound type "<<visitBounds.type());
        }
        return 0.;
   }
#endif
}
