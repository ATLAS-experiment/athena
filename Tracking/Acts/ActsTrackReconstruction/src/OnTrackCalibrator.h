/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ONTRACKCALIBRATOR_H
#define ONTRACKCALIBRATOR_H


#include <GaudiKernel/ToolHandle.h>

#include "MeasurementCalibrator.h"

namespace ActsTrk {

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

    PixelCalibrator pixel_calibrator;
    StripCalibrator strip_calibrator;

    static OnTrackCalibrator
    NoCalibration(const ActsTrk::IActsToTrkConverterTool &converter_tool,
		  const TrackingSurfaceHelper &surface_helper);

    OnTrackCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool,
		      const TrackingSurfaceHelper &surface_helper,
		      const ToolHandle<IOnTrackCalibratorTool<traj_t>> &pixelTool,
		      const ToolHandle<IOnTrackCalibratorTool<traj_t>> &stripTool);

    void calibrate(const Acts::GeometryContext& geoctx,
		   const Acts::CalibrationContext& cctx,
		   const Acts::SourceLink& link,
		   TrackStateProxy state) const;

private:

    // Support the no-calibration case
    template <std::size_t Dim, typename Cluster>
    std::pair<xAOD::MeasVector<Dim>, xAOD::MeasMatrix<Dim>>
    passthrough(const Acts::GeometryContext& gctx,
		const Acts::CalibrationContext& /*cctx*/,
		const Cluster& cluster,
		const TrackStateProxy& state) const;

    // Helper to locate surfaces
    const TrackingSurfaceHelper *m_surfaceHelper;
};

} // namespace ActsTrk

#include "OnTrackCalibrator.icc"

#endif
