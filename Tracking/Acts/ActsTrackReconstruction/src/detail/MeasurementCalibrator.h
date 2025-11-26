/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MEASUREMENTCALIBRATOR2_H
#define MEASUREMENTCALIBRATOR2_H

#include "Acts/EventData/Types.hpp"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/HGTDCluster.h"

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
      MeasurementParameterMap() {
         // @TODO get mapping from converter tool ?
         std::vector<unsigned int> pixel_vol {16, 15, 9, 20, 19, 18, 10, 14, 13,  8};
         for (unsigned int vol_id : pixel_vol) {
            setMeasurementTypeForVolumeId(vol_id, xAOD::UncalibMeasType::PixelClusterType );
         }
         std::vector<unsigned int> strip_vol {23, 22, 24};
         for (unsigned int vol_id : strip_vol) {
            setMeasurementTypeForVolumeId(vol_id, xAOD::UncalibMeasType::StripClusterType );
         }
         std::vector<unsigned int> hgtd_vol {2, 25};
         for (unsigned int vol_id : hgtd_vol) {
            setMeasurementTypeForVolumeId(vol_id, xAOD::UncalibMeasType::HGTDClusterType );
         }
      }

      template <std::size_t DIM>
      Acts::SubspaceIndices<DIM> parameterMap([[maybe_unused]] const Acts::GeometryContext&,
                                     [[maybe_unused]] const Acts::CalibrationContext&,
                                     const Acts::Surface &surface) const {
         // @TODO make interface measurement type aware ?
         if constexpr(DIM==3) {
            assert( measurementTypeFromVolumeId(surface.geometryId().volume()) == xAOD::UncalibMeasType::HGTDClusterType );
            return s_hgtdSubspaceIndices;
         }
         else if constexpr(DIM==2) {
            assert( measurementTypeFromVolumeId(surface.geometryId().volume()) == xAOD::UncalibMeasType::PixelClusterType );
            return s_pixelSubspaceIndices;
         }
         else if constexpr(DIM==1) {
            assert( measurementTypeFromVolumeId(surface.geometryId().volume()) == xAOD::UncalibMeasType::StripClusterType );
            auto boundType = surface.bounds().type();
            const std::size_t projector_idx  = boundType == Acts::SurfaceBounds::eAnnulus;
            return s_stripSubspaceIndices[projector_idx];
         }
         else {
            throw std::runtime_error("Unsupported dimension");
         }

      }

      constexpr static std::array<Acts::SubspaceIndices<1>, 2> s_stripSubspaceIndices = {
        {{Acts::eBoundLoc0}, // normal strip: x -> l0
        {Acts::eBoundLoc1}} // annulus strip: y -> l0
      };
      constexpr static Acts::SubspaceIndices<2> s_pixelSubspaceIndices = {
        Acts::eBoundLoc0, Acts::eBoundLoc1
      };
      constexpr static Acts::SubspaceIndices<3> s_hgtdSubspaceIndices = {
         Acts::eBoundLoc0, Acts::eBoundLoc1, Acts::eBoundTime
      };
   };

   struct MeasurementCalibrator {
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
      using hgtdPos = xAOD::MeasVector<3>;
      using hgtdCov = xAOD::MeasMatrix<3>;
      using HGTDCalibrator = Acts::Delegate<
         std::pair<hgtdPos, hgtdCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const xAOD::HGTDCluster &,
                                       const Acts::BoundTrackParameters &)>;

      PixelCalibrator pixel_postCalibrator;
      StripCalibrator strip_postCalibrator;
      HGTDCalibrator hgtd_postCalibrator;
      PixelCalibrator pixel_preCalibrator;
      StripCalibrator strip_preCalibrator;
      HGTDCalibrator hgtd_preCalibrator;

      MeasurementCalibrator(const IOnBoundStateCalibratorTool *pixelCalibratorTool,
                            const IOnBoundStateCalibratorTool *stripCalibratorTool,
                            const IOnBoundStateCalibratorTool *hgtdCalibratorTool)
      {

         if (pixelCalibratorTool) {
             bool calibrate_after_measurement_selection = pixelCalibratorTool->calibrateAfterMeasurementSelection();
             pixelCalibratorTool->connectPixelCalibrator( calibrate_after_measurement_selection ?
                                                          pixel_postCalibrator : pixel_preCalibrator );
             if (calibrate_after_measurement_selection)
                pixel_preCalibrator.template connect<&MeasurementCalibrator::passthrough<2, xAOD::PixelCluster>>(this);
          } else
             pixel_preCalibrator.template connect<&MeasurementCalibrator::passthrough<2, xAOD::PixelCluster>>(this);

          if (stripCalibratorTool) {
             bool calibrate_after_measurement_selection = stripCalibratorTool->calibrateAfterMeasurementSelection();
             stripCalibratorTool->connectStripCalibrator( calibrate_after_measurement_selection ?
                                                          strip_postCalibrator : strip_preCalibrator );
             if (calibrate_after_measurement_selection)
                strip_preCalibrator.template connect<&MeasurementCalibrator::passthrough<1, xAOD::StripCluster>>(this);
          } else
             strip_preCalibrator.template connect<&MeasurementCalibrator::passthrough<1, xAOD::StripCluster>>(this);


          if (hgtdCalibratorTool) {
             bool calibrate_after_measurement_selection = hgtdCalibratorTool->calibrateAfterMeasurementSelection();
             hgtdCalibratorTool->connectHGTDCalibrator( calibrate_after_measurement_selection ?
                                                        hgtd_postCalibrator : hgtd_preCalibrator );
             if(calibrate_after_measurement_selection)
                hgtd_preCalibrator.template connect<&MeasurementCalibrator::passthrough<3, xAOD::HGTDCluster>>(this);
          } else
             hgtd_preCalibrator.template connect<&MeasurementCalibrator::passthrough<3, xAOD::HGTDCluster>>(this);

      }

      const PixelCalibrator &pixelPostCalibrator() const  { return pixel_postCalibrator; }
      const StripCalibrator &stripPostCalibrator() const { return strip_postCalibrator; }
      const HGTDCalibrator &hgtdPostCalibrator() const { return hgtd_postCalibrator; }
      const PixelCalibrator &pixelPreCalibrator() const { return pixel_preCalibrator; }
      const StripCalibrator &stripPreCalibrator() const { return strip_preCalibrator; }
      const HGTDCalibrator &hgtdPreCalibrator() const { return hgtd_preCalibrator; }   


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
