/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "AthenaKernel/Units.h"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Utilities/AlgebraHelpers.hpp"
#include <Eigen/Core>

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IStripOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IHGTDOnTrackCalibratorTool.h"
#include "ActsInterop/UnitConverters.h"
#include "ActsEvent/TrackContainer.h"

#include "boost/container/static_vector.hpp"

#include <stdexcept>
#include <string>
#include <cassert>
#include <tuple>

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


   template <typename traj_t>
   struct MeasurementCalibrator {
      using PixelPos = xAOD::MeasVector<2>;
      using PixelCov = xAOD::MeasMatrix<2>;
      // @TODO should pass through bound state
      template <typename T_Cluster, std::size_t NDIM>
      using PreCalibratorDelegate = Acts::Delegate<
         std::tuple<xAOD::MeasVector<NDIM>, xAOD::MeasMatrix<NDIM>,unsigned int>(const Acts::GeometryContext&,
                                                                                 const Acts::CalibrationContext&,
                                                                                 const Acts::Surface&,
                                                                                 const T_Cluster &,
                                                                                 const Acts::BoundTrackParameters &)>;

      template <typename T_Cluster, std::size_t NDIM>
      using CalibratorDelegate = Acts::Delegate<
         void(const Acts::GeometryContext&,
              const Acts::CalibrationContext&,
              const T_Cluster &,
              typename traj_t::TrackStateProxy &)>;
      
      using StripPos = xAOD::MeasVector<1>;
      using StripCov = xAOD::MeasMatrix<1>;
      using hgtdPos = xAOD::MeasVector<3>;
      using hgtdCov = xAOD::MeasMatrix<3>;

      using PixelPreCalibrator = PreCalibratorDelegate<xAOD::PixelCluster,2>;
      using StripPreCalibrator = PreCalibratorDelegate<xAOD::StripCluster,1>;
      using HGTDPreCalibrator  = PreCalibratorDelegate<xAOD::HGTDCluster,3>;
      using PixelCalibrator = CalibratorDelegate<xAOD::PixelCluster,2>;
      using StripCalibrator = CalibratorDelegate<xAOD::StripCluster,1>;
      using HGTDCalibrator  = CalibratorDelegate<xAOD::HGTDCluster,3>;

      PixelCalibrator pixel_postCalibrator;
      StripCalibrator strip_postCalibrator;
      HGTDCalibrator hgtd_postCalibrator;
      PixelPreCalibrator pixel_preCalibrator;
      StripPreCalibrator strip_preCalibrator;
      HGTDPreCalibrator hgtd_preCalibrator;
      boost::container::static_vector<std::unique_ptr<ClusterCalibratorBase >, 3> m_calibrators;

      template <typename T_CalibratorTool, typename T_PreDelegate, typename T_PostDelegate>
      void connect(const EventContext &ctx,
                   const T_CalibratorTool *calibrator_tool,
                   T_PreDelegate &pre_calibrator,
                   T_PostDelegate &post_calibrator) {
         bool calibrate_after_measurement_selection=true;
         if (calibrator_tool) {
            calibrate_after_measurement_selection = calibrator_tool->calibrateAfterMeasurementSelection();
            auto calibrator = calibrator_tool->createOnTrackCalibrator(ctx);
            if (calibrate_after_measurement_selection) {
               calibrator->connectOnTrackCalibrator( post_calibrator);
            }
            else {
               calibrator->connectCalibrator( pre_calibrator );
            }
            m_calibrators.push_back(std::move(calibrator));
         }
         if (calibrate_after_measurement_selection) {
            using CalibratorBase_t = typename decltype( calibrator_tool->create(ctx) )::element_type;
            pre_calibrator.template connect<&MeasurementCalibrator::passthrough<CalibratorBase_t::ClusterDIM,
                                                                                typename CalibratorBase_t::ClusterType>>(this);
         }
      }

      MeasurementCalibrator(const EventContext &ctx,
                            const ActsTrk::IPixelOnTrackCalibratorTool<traj_t> *pixelCalibratorTool,
                            const ActsTrk::IStripOnTrackCalibratorTool<traj_t> *stripCalibratorTool,
                            const ActsTrk::IHGTDOnTrackCalibratorTool<traj_t> *hgtdCalibratorTool)
      {
         assert( m_calibrators.capacity() >= 3); // if capacity was constexpr should turn into static_assert
         connect(ctx,pixelCalibratorTool,pixel_preCalibrator,pixel_postCalibrator);
         connect(ctx,stripCalibratorTool,strip_preCalibrator,strip_postCalibrator);
         connect(ctx,hgtdCalibratorTool, hgtd_preCalibrator, hgtd_postCalibrator);
      }

      const PixelCalibrator &pixelPostCalibrator() const  { return pixel_postCalibrator; }
      const StripCalibrator &stripPostCalibrator() const { return strip_postCalibrator; }
      const HGTDCalibrator &hgtdPostCalibrator() const { return hgtd_postCalibrator; }
      const PixelPreCalibrator &pixelPreCalibrator() const { return pixel_preCalibrator; }
      const StripPreCalibrator &stripPreCalibrator() const { return strip_preCalibrator; }
      const HGTDPreCalibrator &hgtdPreCalibrator() const { return hgtd_preCalibrator; }


      template <std::size_t Dim, typename Cluster>
      std::tuple<xAOD::MeasVector<Dim>, xAOD::MeasMatrix<Dim>, unsigned int>
      passthrough([[maybe_unused]] const Acts::GeometryContext& gctx,
                  [[maybe_unused]] const Acts::CalibrationContext& cctx,
                  [[maybe_unused]] const Acts::Surface& surface,
                  const Cluster &cluster,
                  const Acts::BoundTrackParameters &) const
      {
         auto ret = std::make_tuple<xAOD::MeasVector<Dim>, xAOD::MeasMatrix<Dim>, unsigned int>(cluster.template localPosition<Dim>(),
                                                                                                cluster.template localCovariance<Dim>(),
                                                                                                0u);
         if constexpr(std::is_same_v<xAOD::HGTDCluster, std::remove_cvref_t<Cluster> >) {
            std::get<0>(ret)(2,0)  = ActsTrk::timeToActs(std::get<0>(ret)(2,0));
            assert(std::get<1>(ret)(2,1)==0. && std::get<1>(ret)(2,0)==0.);
            std::get<1>(ret)(2,2) = ActsTrk::timeCovToActs(std::get<1>(ret)(2,2));
         }
         return ret;
      }

   };

}
#endif
