/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H
#define ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "src/detail/FitterHelperFunctions.h"
#include "src/detail/OnTrackCalibrator.h"

#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
// ACTS
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"

#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
#include "ActsCalibrators/TrkMeasSurfaceAccessor.h"
#include "ActsCalibrators/TrkPrepRawDataSurfaceAcc.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

namespace Trk {
class PrepRawData;
}  // namespace Trk

namespace ActsTrk {



class GlobalChiSquareFitterTool
    : public extends<AthAlgTool, IFitterTool> {
 public:

  using base_class::base_class;
  virtual ~GlobalChiSquareFitterTool() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;

  //! fit a set of xAOD uncalibrated Measurements
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const std::vector<const xAOD::UncalibratedMeasurement*>& clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const Acts::Surface* targetSurface =
          nullptr  // optional target surface - defaults to perigee in global
                   // origin
  ) const override;

  //! Acts seed fit
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const Seed& seed,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const Acts::Surface& targetSurface) const override;


  virtual StatusCode fit(
    const EventContext& ctx,
    const TrackContainer::ConstTrackProxy& track,          
    MutableTrackContainer& trackContainer,
    const Acts::PerigeeSurface& pSurface) const override;

  //! fit a set of source links
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const std::vector<Acts::SourceLink>& sourceLinks,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const Acts::Surface* targetSurface =
          nullptr  // optional target surface - defaults to perigee in global
                   // origin
  ) const override;


    /// Type erased track fitter function.
    using StraightPropagator_t = Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>;
    using CurvedPropagator_t   = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;

    using StraightFitter_t = Acts::Experimental::Gx2Fitter<StraightPropagator_t, MutableTrackStateBackend>;
    using CurvedFitter_t   = Acts::Experimental::Gx2Fitter<CurvedPropagator_t, MutableTrackStateBackend>;

    /** @brief Abbrivation of the configuration to launch the fit  */
    using Gx2FitterOptions_t = Acts::Experimental::Gx2FitterOptions<MutableTrackStateBackend>;
    /** @brief Abbrivation of the fitter extensions */
    using Gx2FitterExtension_t = Acts::Experimental::Gx2FitterExtensions<MutableTrackStateBackend>;
   
  private:
    /** @brief Abrivate the track state proxy */
    using TrackState_t = MutableTrackStateBackend::TrackStateProxy;

    /** @brief Helper method to pack the last information (Calibration, Alignment, B-Field, etc.)
     *         for the fit. The parsed context objects need to prevail the call of the fit
     * @param tgContext: Reference to the geometry context
     * @param mfContext: Reference to the passed magnetic field context
     * @param calContext: Reference to the calibration context
     * @param surface: Target surface on which the fit result is expressed.
     * @param slType: Switch of which source link type shall be used to access the surfaces from
     *                the measurements & for calibration */
    Gx2FitterOptions_t configureFit(const Acts::GeometryContext& tgContext,
                                    const Acts::MagneticFieldContext& mfContext,
                                    const Acts::CalibrationContext& calContext,
                                    const Acts::Surface* surface,
                                    detail::SourceLinkType slType) const;
   
    ToolHandle<IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
    PublicToolHandle<ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
    PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};
    
    ToolHandle<MuonR4::ISpacePointCalibrator> m_muonCalibrator{this, "MuonCalibrationTool", ""};

    ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator{this, "RotCreatorTool", ""};
    
    /** @brief Chi2 cut used by the outlier finder */
    Gaudi::Property<double> m_option_outlierChi2Cut{this, "OutlierChi2Cut", 12.5};
    /** @brief Maximum number of steps per propagation call */
    Gaudi::Property<unsigned> m_option_maxPropagationStep{this, "MaxPropagationStep", 5000};
    /** @brief Number of maximum surfaces to be tried before the navigrator aborts */
    Gaudi::Property<unsigned> m_option_maxNavSurfaces{this, "MaxSurfacesPerNavStep", 100};
    /** @brief Consider particle's energy loss in the fit  */
    Gaudi::Property<bool> m_option_includeELoss{this, "IncludeELoss", true};
    /** @brief Consider multiple scattering of the particle */
    Gaudi::Property<bool> m_option_includeScat{this, "IncludeScattering", true};
    /** @brief Option to toggle whether a straight line fitter shall be used */
    Gaudi::Property<bool> m_doStraightLine{this, "DoStraightLine" , false};
    /** @brief Account for non linear effects from free -> bound jacobian  */
    Gaudi::Property<bool> m_doJacobianCorr{this, "DoFreeToBoundCorrection", false };
    /** @brief Number of iterations a fit may take */
    Gaudi::Property<unsigned> m_nIterMax{this, "MaxIterations", 5};
    /** @brief Pass through calibrator of the Trk::MeasurementBase objects from the TrackState container */
    detail::TrkMeasurementCalibrator m_trkMeasCalibrator{};
    /** @brief Calibrator of the uncalibrated Trk::PrepRawData objects to RIO_OnTrack objects */
    detail::TrkPrepRawDataCalibrator m_prdCalibrator{};
    /** @brief Surface accessor delegate for Trk::MeasurementBase objects */
    detail::TrkMeasSurfaceAccessor m_trkMeasSurfAcc{};
    /** @brief Surface accessor delegate for Trk::PrepRawData objects */
    detail::TrkPrepRawDataSurfaceAcc m_prdSurfaceAcc{};
    /** @brief Surface accessor delegate for xAOD::UncalibratedMeasurement objects */
    detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc{};
    /** @brief Calibrator for the uncalibrated xAOD::UnCalibratedMeasurement objects */
    detail::xAODUncalibMeasCalibrator m_uncalibMeasCalibrator{};
    /** @brief Calibrator of the ID / ITk measurements */
    using xAODItkCalibrator_t = detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend> ;
    xAODItkCalibrator_t m_idCalibrator{};

    /** @brief Array of all configured fitter extensions depending on which source link type is in use */
    static constexpr unsigned s_nExtensions = static_cast<unsigned>(detail::SourceLinkType::nTypes);
    std::array<Gx2FitterExtension_t, s_nExtensions>  m_gx2fExtensions{};
    /** @brief The underlying curved Acts fitter */
    std::unique_ptr<CurvedFitter_t> m_fitter{nullptr};
    /** @brief The underlying straight line Acts fitter */
    std::unique_ptr<StraightFitter_t> m_slFitter{nullptr};

    detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

    /// Private access to the logger
    const Acts::Logger& logger() const { return *m_logger; }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;


};

}  // namespace ActsTrk
#endif
