/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#   include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#   include <Acts/Geometry/VolumeBounds.hpp>
#   include <Acts/Geometry/CuboidVolumeBounds.hpp>
#   include <Acts/Geometry/TrapezoidVolumeBounds.hpp>


namespace MuonGMR4 {
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
}
#endif
