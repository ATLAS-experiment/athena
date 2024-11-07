/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H

#include "GaudiKernel/ToolHandle.h"

#include "src/detail/MeasurementCalibrator.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

namespace ActsTrk::detail {

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

    PixelCalibrator pixelCalibrator;
    StripCalibrator stripCalibrator;

    static OnTrackCalibrator
    NoCalibration(const Acts::TrackingGeometry &trackingGeometry,
                  const ActsTrk::DetectorElementToActsGeometryIdMap &detectorElementToGeoId);

    OnTrackCalibrator(const Acts::TrackingGeometry &trackingGeometry,
                      const ActsTrk::DetectorElementToActsGeometryIdMap &detectorElementToGeoId,
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
    const Acts::TrackingGeometry *m_trackingGeometry {nullptr};
    const ActsTrk::DetectorElementToActsGeometryIdMap *m_detectorElementToGeoId {nullptr};
};

} // namespace ActsTrk

#include "src/detail/OnTrackCalibrator.icc"

#endif
