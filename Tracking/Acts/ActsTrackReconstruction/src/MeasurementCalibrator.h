/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MEASUREMENTCALIBRATOR_H
#define MEASUREMENTCALIBRATOR_H

#include "Acts/EventData/Types.hpp"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Utilities/AlgebraHelpers.hpp"
#include <Eigen/Core>

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"

#include <stdexcept>
#include <string>
#include <cassert>

namespace ActsTrk {
class MeasurementCalibratorBase
{
protected:
   constexpr static std::array<Acts::BoundSubspaceIndices, 2> s_stripSubspaceIndices = {
     Acts::BoundSubspaceIndices{Acts::eBoundLoc0}, // normal strip: x -> l0
     Acts::BoundSubspaceIndices{Acts::eBoundLoc1} // annulus strip: y -> l0
   };
   constexpr static Acts::BoundSubspaceIndices s_pixelSubspaceIndices = {
    Acts::eBoundLoc0, Acts::eBoundLoc1
   };

public:
   MeasurementCalibratorBase() = default;

   template <typename state_t>
   inline void setProjector(xAOD::UncalibMeasType measType,
                            Acts::SurfaceBounds::BoundsType boundType,
                            state_t &trackState ) const {
     switch (measType) {
       case xAOD::UncalibMeasType::StripClusterType: {
         const std::size_t projector_idx  = boundType == Acts::SurfaceBounds::eAnnulus;
         trackState.setBoundSubspaceIndices(s_stripSubspaceIndices[projector_idx]);
         break;
       }
       case xAOD::UncalibMeasType::PixelClusterType: {
         trackState.setBoundSubspaceIndices(s_pixelSubspaceIndices);
         break;
       }
       default:
         throw std::domain_error("Can only handle measurement type pixel or strip");
     }
   }

   template <size_t Dim, typename pos_t, typename cov_t, typename state_t>
   inline void setState(xAOD::UncalibMeasType measType,
			const pos_t& locpos,
			const cov_t& cov,
			Acts::SurfaceBounds::BoundsType boundType,
			state_t &trackState) const {
       trackState.allocateCalibrated(Dim);
       setProjector(measType, boundType, trackState);
       trackState.template calibrated<Dim>() = locpos.template cast<Acts::ActsScalar>();
       trackState.template calibratedCovariance<Dim>() = cov.template cast<Acts::ActsScalar>();
   }


   template <class measurement_t, typename trajectory_t>
   inline void setStateFromMeasurement(const measurement_t &measurement,
                                       Acts::SurfaceBounds::BoundsType bound_type,
                                       typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy &trackState ) const {
       switch (measurement.type()) {
       case (xAOD::UncalibMeasType::StripClusterType): {
	   setState<1>(
	       measurement.type(),
	       measurement.template localPosition<1>(),
	       measurement.template localCovariance<1>().template topLeftCorner<1, 1>(),
	       bound_type,
	       trackState);
	   break;
       }
       case (xAOD::UncalibMeasType::PixelClusterType): {
	   setState<2>(
	       measurement.type(),
	       measurement.template localPosition<2>(),
	       measurement.template localCovariance<2>().template topLeftCorner<2, 2>(),
	       bound_type,
	       trackState);
	   break;
       }
       default:
         throw std::domain_error("Can only handle measurement type pixel or strip");
      }
   }

};

class TrkMeasurementCalibrator : public MeasurementCalibratorBase {
public:
   class MeasurementAdapter {
   public:
      MeasurementAdapter(const Trk::MeasurementBase &measurement) : m_measurement(&measurement) {}
      xAOD::UncalibMeasType type() const {
         switch (m_measurement->localParameters().dimension()) {
         case 1: {
            return xAOD::UncalibMeasType::StripClusterType;
         }
         case 2: {
            return xAOD::UncalibMeasType::PixelClusterType;
         }
         default: {
            return xAOD::UncalibMeasType::Other;
         }
         }
      }
      template <std::size_t DIM>
      inline const Trk::LocalParameters& localPosition() const {
         assert( m_measurement && DIM == m_measurement->localParameters().dimension());
         return m_measurement->localParameters();
      }
      template <std::size_t DIM>
      inline const Amg::MatrixX &localCovariance() const {
         assert( m_measurement && DIM == m_measurement->localParameters().dimension());
         return m_measurement->localCovariance();
      }
   private:
      const Trk::MeasurementBase *m_measurement;
   };

   TrkMeasurementCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool)
      : m_converterTool(&converter_tool) {}

   template <typename trajectory_t>
   void calibrate([[maybe_unused]] const Acts::GeometryContext &gctx,
                   [[maybe_unused]] const Acts::CalibrationContext & cctx,
                   const Acts::SourceLink& sl,
                   typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState) const {
      auto sourceLink = sl.template get<ATLASSourceLink>();
      trackState.setUncalibratedSourceLink(Acts::SourceLink{sl});
      assert(sourceLink);
      const Acts::Surface &surface = this->m_converterTool->trkSurfaceToActsSurface(sourceLink->associatedSurface());
      this->setStateFromMeasurement<MeasurementAdapter, trajectory_t>(MeasurementAdapter(*sourceLink),
                                    surface.bounds().type(),
                                    trackState);
   }
private:
   const ActsTrk::IActsToTrkConverterTool *m_converterTool;
};
}
#endif
