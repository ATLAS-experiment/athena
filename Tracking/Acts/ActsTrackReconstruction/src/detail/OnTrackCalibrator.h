/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H

#include "GaudiKernel/ToolHandle.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "ActsCalibration/MeasurementCalibratorBase.h"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/HGTDCluster.h"
namespace ActsTrk::detail {

/** @brief Inner detector / ITk calibrator implementation used in the KalmanFilterTool */
template <typename traj_t>
class OnTrackCalibrator : MeasurementCalibratorBase {
public:
    using TrackStateProxy = typename Acts::MultiTrajectory<traj_t>::TrackStateProxy;

    using PixelPos = xAOD::MeasVector<2>;
    using PixelCov = xAOD::MeasMatrix<2>;
    using PixelCalibrator = Acts::Delegate<
	std::pair<PixelPos, PixelCov>(const Acts::GeometryContext&,
				      const Acts::CalibrationContext&,
				      const xAOD::PixelCluster&,
				      const TrackStateProxy&)>;

    using StripPos = xAOD::MeasVector<1>;
    using StripCov = xAOD::MeasMatrix<1>;
    using StripCalibrator = Acts::Delegate<
	std::pair<StripPos, StripCov>(const Acts::GeometryContext&,
				      const Acts::CalibrationContext&,
				      const xAOD::StripCluster&,
				      const TrackStateProxy&)>;

    using HgtdPos = xAOD::MeasVector<3>;
    using HgtdCov = xAOD::MeasMatrix<3>;
    using HGTDCalibrator = Acts::Delegate<
  	std::pair<HgtdPos, HgtdCov>(const Acts::GeometryContext&,
              			    const Acts::CalibrationContext&,
              			    const xAOD::HGTDCluster &,
              			    const TrackStateProxy&)>;


    PixelCalibrator pixelCalibrator;
    StripCalibrator stripCalibrator;
    HGTDCalibrator hgtdCalibrator;
    /** @brief Constructs a calibrator which copies the local position & covariance of the ITk measurements 
     *         onto the track state
     * @param trackGeoTool: Pointer to a valid tracking geometry tool to associate the surfaces to the measurements */
    static OnTrackCalibrator
    NoCalibration(const IActsTrackingGeometryTool* trackGeoTool);
    /** @brief Empty default constructor. Surface look will fail. */
    OnTrackCalibrator() = default;
    /** @brief Standard cosntructor which activates the calibration of the ITk & HGTD measurements 
     *         based on the best track predicition. It takes the configured instance to the TrackingGeometryTool
     *         and then for each silicon measurement type a calibration tool handle. There's also the possibility
     *         to pass an empty tool, then the information from the measurement is directly copied onto the track state.     *         
     *  @param trackGeoTool: Pointer to the tracking geometry tool to access the needed surfaces
     *                        during the calibration
     *  @param pixelTool: Reference to a (configured) calibration tool responsible for the PixelCluster measurements
     *  @param stripTool: Reference to a (configured) calibration tool responsible for the ITk strip measurements
     *  @param hdtdTool: Reference to a  (configured) calibration tool responsible for the HGTD strip measurements */
    OnTrackCalibrator(const IActsTrackingGeometryTool* trackGeoTool,
                      const ToolHandle<IOnTrackCalibratorTool<traj_t>> &pixelTool,
                      const ToolHandle<IOnTrackCalibratorTool<traj_t>> &stripTool,
                      const ToolHandle<IOnTrackCalibratorTool<traj_t>> &hgtdTool);

    /** @brief Function that's hooked to the calibration delegate of the implemented Acts fitters
     *  @param geoctx: The geometry context to fetch the local -> global transformations for the surfaces
     *  @param cctx: Calibration context which is a packed pointer to the current ATLAS EventContext
     *  @param link: Sourcelink to the actual measurement to calibrate
     *  @param state: Proxy to the track state onto which the calibrated information is copied. */
    void calibrate(const Acts::GeometryContext& geoctx,
		   const Acts::CalibrationContext& cctx,
		   const Acts::SourceLink& link,
		   TrackStateProxy state) const;

private:
    /** @brief Helper class to access the Acts surfaces */
    xAODUncalibMeasSurfAcc m_surfAcc{};
    // Support the no-calibration case
    template <std::size_t Dim, typename Cluster>
    std::pair<xAOD::MeasVector<Dim>, xAOD::MeasMatrix<Dim>>
    passthrough(const Acts::GeometryContext& gctx,
		const Acts::CalibrationContext& /*cctx*/,
		const Cluster& cluster,
		const TrackStateProxy& state) const;
};

} // namespace ActsTrk

#include "src/detail/OnTrackCalibrator.icc"

#endif
