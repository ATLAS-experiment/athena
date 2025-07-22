/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_VERSION_AUXILLARYMEASUREMENT_V1_H
#define XAODAUXILLARYMEASUREMENT_VERSION_AUXILLARYMEASUREMENT_V1_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "xAODTracking/TrackSurface.h"

#include "AthLinks/ElementLink.h"
#include "CxxUtils/CachedValue.h"
#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "Acts/Acts/Surfaces/Surface.hpp"



namespace xAOD {
    /** @brief Implementation of an uncalibrated AuxiliaryMeasurement which may serve as 
     *         an external constraint in the track fit. The pseudo measurement is always
     *         expressed at the origin of the associated surface. */
    class AuxiliaryMeasurement_v1 : public UncalibratedMeasurement_v1 {
        public:
            /** @brief Returns the measurement type */
            UncalibMeasType type() const override final{
                return UncalibMeasType::Other;
            }
            /** @brief Default constructor */
            AuxiliaryMeasurement_v1() = default;
            /** @brief number of dimensions */
            virtual unsigned numDimensions() const override final;
            /** @brief Surfaces are passed via xAOD::TrackSurfaces which are 
             *         internally converted by the measurement class into Acts
             *         surfaces. The xAOD::TrackSurfaces can be persitified later */
            using SurfLink_t = ElementLink<xAOD::TrackSurfaceContainer>;
            /** @brief Returns the link to the associated xAOD::TrackSurface.   
             *         It maybe invalid if not set before */
            const SurfLink_t& surfaceLink() const;

            using SurfacePtr_t = std::shared_ptr<const Acts::Surface>;
            /** @brief Returns the reference to the Acts::Surface */
            const SurfacePtr_t& surface() const;
            /** @brief Associates a surface with the Auxiliary measurement together
             *         with its persitifiable surface link
             *  @param surfPtr: Pointer to the transient Acts::Surface
             *  @param surfLink: Link to the persitifiable surface */
            void setSurface(const SurfacePtr_t& surfPtr,
                            SurfLink_t&& surfLink);


            /** @brief Use the calibration projector */
            using ProjectorType = ActsTrk::detail::MeasurementCalibratorBase::ProjectorType;
            /** @brief Returns the calibration projector */
            ProjectorType calibProjector() const;
            /** @brief Sets the calibration projector */
            void setProjector(ProjectorType proj);

        private:
            CxxUtils::CachedValue<SurfacePtr_t> m_surface{};
    };
}
#endif