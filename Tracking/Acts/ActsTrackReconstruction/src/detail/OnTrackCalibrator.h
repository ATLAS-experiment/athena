/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_ONTRACKCALIBRATOR_H

#include "GaudiKernel/ToolHandle.h"
#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IStripOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IHGTDOnTrackCalibratorTool.h"
#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/HGTDCluster.h"
#include "boost/container/static_vector.hpp"
#include "ActsInterop/UnitConverters.h"

namespace ActsTrk::detail {

/** @brief Inner detector / ITk calibrator implementation used in the KalmanFilterTool */
template <typename traj_t>
class OnTrackCalibrator : MeasurementCalibratorBase {
public:
    using TrackStateProxy = typename Acts::MultiTrajectory<traj_t>::TrackStateProxy;

    using PixelPos = xAOD::MeasVector<2>;
    using PixelCov = xAOD::MeasMatrix<2>;

    using StripPos = xAOD::MeasVector<1>;
    using StripCov = xAOD::MeasMatrix<1>;

    using HgtdPos = xAOD::MeasVector<3>;
    using HgtdCov = xAOD::MeasMatrix<3>;

    template <typename T_Cluster, std::size_t NDIM>
    using OnTrackCalibratorDelegate = Acts::Delegate<
       void(const Acts::GeometryContext&,
            const Acts::CalibrationContext&,
            const T_Cluster &,
            TrackStateProxy&)>;
    using PixelCalibrator = OnTrackCalibratorDelegate<const xAOD::PixelCluster,2>;
    using StripCalibrator = OnTrackCalibratorDelegate<const xAOD::StripCluster,1>;
    using HGTDCalibrator = OnTrackCalibratorDelegate<const xAOD::HGTDCluster,3>;

    PixelCalibrator pixelCalibrator;
    StripCalibrator stripCalibrator;
    HGTDCalibrator hgtdCalibrator;
    /** @brief Constructs a calibrator which copies the local position & covariance of the ITk measurements 
     *         onto the track state
     * @param trackGeoSvc: Pointer to a valid tracking geometry service to associate the surfaces to the measurements */
    static OnTrackCalibrator
    NoCalibration(const ActsTrk::ITrackingGeometrySvc* trackGeoSvc) {
       return OnTrackCalibrator(trackGeoSvc);
    }

    /** @brief Empty default constructor. Surface look will fail. */
    OnTrackCalibrator() = default;
protected:
    /** @brief create a "NoCalibration" on track calibrator for all measurement types.*/
    OnTrackCalibrator(const ActsTrk::ITrackingGeometrySvc* trackGeoSvc);
public:
    /** @brief Standard cosntructor which activates the calibration of the ITk & HGTD measurements 
     *         based on the best track predicition. It takes the TrackingGeometryService
     *         and then for each silicon measurement type a calibration tool handle. There's also the possibility
     *         to pass an empty tool, then the information from the measurement is directly copied onto the track state.     *         
     *  @param trackGeoSvc: Pointer to the tracking geometry service to access the needed surfaces
     *                      during the calibration
     *  @param pixelTool: Reference to a (configured) calibration tool responsible for the PixelCluster measurements
     *  @param stripTool: Reference to a (configured) calibration tool responsible for the ITk strip measurements
     *  @param hdtdTool: Reference to a  (configured) calibration tool responsible for the HGTD strip measurements */
    OnTrackCalibrator(const EventContext &ctx,
                      const ActsTrk::ITrackingGeometrySvc* trackGeoSvc,
                      const ToolHandle<IPixelOnTrackCalibratorTool<traj_t>> &pixelTool,
                      const ToolHandle<IStripOnTrackCalibratorTool<traj_t>> &stripTool,
                      const ToolHandle<IHGTDOnTrackCalibratorTool<traj_t>> &hgtdTool);

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

    /** @brief all the calibrator objects used and owned by this on track calibrator.*/
    boost::container::static_vector<std::unique_ptr<ClusterCalibratorBase >, 3> m_calibrators;

    // Support the no-calibration case
    template <std::size_t Dim, typename Cluster>
    void passthrough(const Acts::GeometryContext& gctx,
                     const Acts::CalibrationContext& /*cctx*/,
                     const Cluster& cluster,
                     TrackStateProxy& state) const;

    // connect passThrough calibrator for a certain measurement type
    template <typename T_CalibratorToolHandle, typename T_Delegate>
    void connectPassThrough(T_Delegate &delegate);

    // connect the calibrator provided by the given tool or the passthrough calibrator
    // depending on the enable state of the tool.
    template<typename T_CalibratorToolHandle, typename T_Delegate>
    void connect(const EventContext &ctx,
                 const T_CalibratorToolHandle &calibrator_tool,
                 T_Delegate &delegate);

};

} // namespace ActsTrk

#include "src/detail/OnTrackCalibrator.icc"

#endif
