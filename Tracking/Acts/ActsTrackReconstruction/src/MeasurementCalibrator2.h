/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MEASUREMENTCALIBRATOR2_H
#define MEASUREMENTCALIBRATOR2_H

#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Utilities/AlgebraHelpers.hpp"
#include <Eigen/Core>

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsToolInterfaces/IOnBoundStateCalibratorTool.h"

#include <stdexcept>
#include <string>
#include <cassert>

namespace ActsTrk {
   // helper to create map from nound track parameters to measurements
   struct MeasurementParameterMap {
      template <std::size_t N>
      using ParameterMap = std::array<unsigned char, N>;

      static constexpr ParameterMap<1> makeStripProjector(bool annulus_strip) {
         if (annulus_strip) {
            return ParameterMap<1>{static_cast<unsigned char>(Acts::eBoundLoc1)};
         } else {
            return ParameterMap<1>{static_cast<unsigned char>(Acts::eBoundLoc0)};
         }
      }
      // create strip projector bitsets for normal and annulus bounds
      static constexpr std::array<ParameterMap<1>,2> makeStripProjectorArray() {
         return std::array<ParameterMap<1>,2>{makeStripProjector(false), makeStripProjector(true) };
      }

      // create pixel projector bitsets
      static constexpr ParameterMap<2> makePixelProjector() {
         return ParameterMap<2>{static_cast<unsigned char>(Acts::eBoundLoc0), static_cast<unsigned char>(Acts::eBoundLoc1)};
      }

      std::array<unsigned char, 128> m_volumeIdToMeasurementType{};
      xAOD::UncalibMeasType measurementTypeFromVolumeId(unsigned int volume_id) const {
         unsigned char shift = (volume_id%2) ? 4 : 0;
         unsigned char idx = volume_id/2;
         return static_cast<xAOD::UncalibMeasType>((m_volumeIdToMeasurementType[idx] >> shift) & 0xf);
      }
      void setMeasurementTypeForVolumeId(unsigned int volume_id, xAOD::UncalibMeasType type) {
         static_assert( static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes) <  16u );
         unsigned char shift = (volume_id%2) ? 4 : 0;
         unsigned char idx = volume_id/2;
         m_volumeIdToMeasurementType[idx] |= ((static_cast<unsigned int>(type) & 0xf) << shift);
      }
      MeasurementParameterMap()
         : m_stripProjector( makeStripProjectorArray()),
           m_pixelProjector( makePixelProjector() )
      {
         // @TODO get mapping from converter tool ?
         std::vector<unsigned int> pixel_vol {16, 15, 9, 20, 19, 18, 10, 14, 13,  8};
         for (unsigned int vol_id : pixel_vol) {
            setMeasurementTypeForVolumeId(vol_id, xAOD::UncalibMeasType::PixelClusterType );
         }
         std::vector<unsigned int> strip_vol {23, 22, 24};
         for (unsigned int vol_id : strip_vol) {
            setMeasurementTypeForVolumeId(vol_id, xAOD::UncalibMeasType::StripClusterType );
         }
      }

      template <std::size_t DIM>
      ParameterMap<DIM> parameterMap([[maybe_unused]] const Acts::GeometryContext&,
                                     [[maybe_unused]] const Acts::CalibrationContext&,
                                     const Acts::Surface &surface) const {
         // @TODO make interface measurement type aware ?
         if constexpr(DIM==2) {
            assert( measurementTypeFromVolumeId(surface.geometryId().volume()) == xAOD::UncalibMeasType::PixelClusterType );
            return m_pixelProjector;
         }
         else if constexpr(DIM==1) {
            assert( measurementTypeFromVolumeId(surface.geometryId().volume()) == xAOD::UncalibMeasType::StripClusterType );
            auto boundType = surface.bounds().type();
            const std::size_t projector_idx  = boundType == Acts::SurfaceBounds::eAnnulus;
            return m_stripProjector[projector_idx];
         }
         else {
            throw std::runtime_error("Unsupported dimension");
         }

      }
      std::array<ParameterMap<1>,2> m_stripProjector; // strip projector for normal and annulus bounds
      ParameterMap<2>               m_pixelProjector;

   };

   struct MeasurementCalibrator2 {
      using PixelPos = xAOD::MeasVector<2>;
      using PixelCov = xAOD::MeasMatrix<2>;
      // @TODO should pass through bound state
      using PixelCalibrator = Acts::Delegate<
         std::pair<PixelPos, PixelCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const xAOD::PixelCluster &,
                                       const Acts::BoundTrackParameters &)>;

      using StripPos = xAOD::MeasVector<1>;
      using StripCov = xAOD::MeasMatrix<1>;
      using StripCalibrator = Acts::Delegate<
         std::pair<StripPos, StripCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const xAOD::StripCluster &,
                                       const Acts::BoundTrackParameters &)>;
      PixelCalibrator pixel_postCalibrator;
      StripCalibrator strip_postCalibrator;
      PixelCalibrator pixel_preCalibrator;
      StripCalibrator strip_preCalibrator;

      MeasurementCalibrator2(const IOnBoundStateCalibratorTool *pixelTool)
      {
         // @TODO add support for real calibrators

         bool calibrate_after_measurement_selection = true;
         if (pixelTool) {
            calibrate_after_measurement_selection = pixelTool->calibrateAfterMeasurementSelection();
            pixelTool->connectPixelCalibrator( calibrate_after_measurement_selection ? pixel_postCalibrator : pixel_preCalibrator );
         }
         if (calibrate_after_measurement_selection) {
            pixel_preCalibrator.template connect<&MeasurementCalibrator2::passthrough<2, xAOD::PixelCluster>>(this);
         }
         strip_preCalibrator.template connect<&MeasurementCalibrator2::passthrough<1, xAOD::StripCluster>>(this);
      }

      const PixelCalibrator &pixelPostCalibrator() const  { return pixel_postCalibrator; }
      const StripCalibrator &stripPostCalibrator() const { return strip_postCalibrator; }
      const PixelCalibrator &pixelPreCalibrator() const { return pixel_preCalibrator; }
      const StripCalibrator &stripPreCalibrator() const { return strip_preCalibrator; }


      template <std::size_t Dim, typename Cluster>
      std::pair<xAOD::MeasVector<Dim>, xAOD::MeasMatrix<Dim>>
      passthrough([[maybe_unused]] const Acts::GeometryContext& gctx,
                  [[maybe_unused]] const Acts::CalibrationContext& cctx,
                  const Cluster &cluster,
                  const Acts::BoundTrackParameters &) const
      {
         return std::make_pair(cluster.template localPosition<Dim>(),
                               cluster.template localCovariance<Dim>());
      }
   };

}
#endif
