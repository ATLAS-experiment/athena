/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H
#define ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "src/detail/FitterHelperFunctions.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "ActsCalibration/TrkMeasurementCalibrator.h"
#include "ActsCalibration/TrkPrepRawDataCalibrator.h"
#include "ActsCalibration/TrkMeasSurfaceAccessor.h"
#include "ActsCalibration/TrkPrepRawDataSurfaceAcc.h"
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibration/xAODUncalibMeasCalibrator.h"

// STL
#include <cmath>   //std::abs
#include <limits>  //for numeric_limits
#include <memory>  //unique_ptr
#include <string>

#include "ActsToolInterfaces/IFitterTool.h"



namespace Trk {
class Track;
class PrepRawData;
}  // namespace Trk

namespace ActsTrk {



class GlobalChiSquareFitterTool
    : public extends<AthAlgTool, Trk::ITrackFitter, IFitterTool> {
 public:

  using base_class::base_class;
  virtual ~GlobalChiSquareFitterTool() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;

  //! refit a track
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::Track&,
      const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! fit a set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::PrepRawDataSet&,
      const Trk::TrackParameters&,
      const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! fit a set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::MeasurementSet&,
      const Trk::TrackParameters&,
      const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! extend a track fit including a new set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::Track&, const Trk::PrepRawDataSet&,
      const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! fit a set of xAOD uncalibrated Measurements
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const std::vector<ATLASUncalibSourceLink>& clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const Acts::Surface* targetSurface =
          nullptr  // optional target surface - defaults to perigee in global
                   // origin
  ) const override;

  //! extend a track fit including a new set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::Track&, const Trk::MeasurementSet&,
      const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! combined track fit
  virtual std::unique_ptr<Trk::Track> fit(
      const EventContext& ctx, const Trk::Track& intrk1,
      const Trk::Track& intrk2, const Trk::RunOutlierRemoval runOutlier = false,
      const Trk::ParticleHypothesis matEffects =
          Trk::nonInteracting) const override;

  //! Acts seed fit
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const Seed& seed,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext) const override;


  virtual StatusCode fit(
    const EventContext& ctx,
        const TrackContainer::ConstTrackProxy& track,          
    MutableTrackContainer& trackContainer) const override;


    /// Type erased track fitter function.
    using Fitter = Acts::Experimental::Gx2Fitter<Acts::Propagator<Acts::SympyStepper, Acts::Navigator>,
                                                 MutableTrackStateBackend>;
    /** @brief Abbrivation of the configuration to launch the fit  */
    using Gx2FitterOptions_t = Acts::Experimental::Gx2FitterOptions<MutableTrackStateBackend>;
    /** @brief Abbrivation of the fitter extensions */
    using Gx2FitterExtension_t = Acts::Experimental::Gx2FitterExtensions<MutableTrackStateBackend>;
  private:
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
   
   ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
   PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
   ToolHandle<IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};


  // the settable job options
  Gaudi::Property<double> m_option_outlierChi2Cut{
      this, "OutlierChi2Cut", 12.5, "Chi2 cut used by the outlier finder"};
  Gaudi::Property<int> m_option_maxPropagationStep{
      this, "MaxPropagationStep", 5000,
      "Maximum number of steps for one propagate call"};
  Gaudi::Property<double> m_option_seedCovarianceScale{
      this, "SeedCovarianceScale", 100.,
      "Scale factor for the input seed covariance when doing refitting"};

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
  /** @brief Array of all configured fitter extensions depending on which source link type is in use */
  static constexpr unsigned s_nExtensions = static_cast<unsigned>(detail::SourceLinkType::nTypes);
  std::array<Gx2FitterExtension_t, s_nExtensions>  m_gx2fExtensions{};

  std::unique_ptr<Fitter> m_fitter{nullptr};

  detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// Private access to the logger
  const Acts::Logger& logger() const { return *m_logger; }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator{this, "RotCreatorTool", ""};

  // Gaudi Property to choose from PRD or ROT measurement ReFit
  Gaudi::Property<bool> m_doReFitFromPRD{this, "DoReFitFromPRD", false,
                                         "Do Refit From PRD instead of ROT"};
};

}  // namespace ActsTrk
#endif
