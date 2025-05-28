/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_KALMANFITTERTOOL_H
#define ACTSTRACKRECONSTRUCTION_KALMANFITTERTOOL_H

#include "src/detail/FitterHelperFunctions.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkFitterInterfaces/ITrackFitter.h"

#include "TrkPrepRawData/PrepRawData.h"

#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"


// PACKAGE

#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"

#include "ActsCalibration/CalibrationContext.h"
#include "ActsCalibration/TrkMeasSurfaceAccessor.h"
#include "ActsCalibration/TrkPrepRawDataCalibrator.h"
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibration/TrkPrepRawDataSurfaceAcc.h"
#include "src/detail/OnTrackCalibrator.h"
// STL
#include <string>
#include <memory>//unique_ptr
#include <limits>//for numeric_limits
#include <cmath> //std::abs


#include "ActsToolInterfaces/IFitterTool.h"

class EventContext;

namespace Trk{
  class Track;
  class PrepRawData;
}

namespace ActsTrk {

class KalmanFitterTool
  : public extends<AthAlgTool, Trk::ITrackFitter, ActsTrk::IFitterTool> { 
public:

  using base_class::base_class;
  virtual ~KalmanFitterTool() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;

  //! refit a track
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! fit a set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::PrepRawDataSet&,
    const Trk::TrackParameters&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! fit a set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::MeasurementSet&,
    const Trk::TrackParameters&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! extend a track fit including a new set of PrepRawData objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::PrepRawDataSet&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;
  
  //! fit a set of xAOD uncalibrated Measurements
  virtual  
      std::unique_ptr< ActsTrk::MutableTrackContainer >
      fit(const std::vector<ActsTrk::ATLASUncalibSourceLink> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,      
      const Acts::Surface* targetSurface = nullptr  // optional target surface - defaults to perigee in global origin
      ) const override;

  //! extend a track fit including a new set of MeasurementBase objects
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track&,
    const Trk::MeasurementSet&,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! combined track fit
  virtual std::unique_ptr<Trk::Track> fit(
    const EventContext& ctx,
    const Trk::Track& intrk1,
    const Trk::Track& intrk2,
    const Trk::RunOutlierRemoval runOutlier = false,
    const Trk::ParticleHypothesis matEffects = Trk::nonInteracting) const override;

  //! Acts seed fit
  virtual
    std::unique_ptr< ActsTrk::MutableTrackContainer >
    fit(const ActsTrk::Seed &seed,
        const Acts::BoundTrackParameters& initialParams,
        const Acts::GeometryContext& tgContext,
        const Acts::MagneticFieldContext& mfContext,
        const Acts::CalibrationContext& calContext) const override;


    virtual StatusCode fit(
        const EventContext& ctx,
                const ActsTrk::TrackContainer::ConstTrackProxy& track,          
        ActsTrk::MutableTrackContainer& trackContainer) const override;
  
  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

    /** @brief Abbrivation of the fitter extensions */
    using FitterExtension_t = Acts::KalmanFitterExtensions<MutableTrackStateBackend>;
    /** @brief Abbrivation of the configuration to launch the fit  */
    using FitterOptions_t = Acts::KalmanFitterOptions<MutableTrackStateBackend>;
    /** @brief Helper method to pack the last information (Calibration, Alignment, B-Field, etc.)
     *         for the fit. The parsed context objects need to prevail the call of the fit
     * @param tgContext: Reference to the geometry context
     * @param mfContext: Reference to the passed magnetic field context
     * @param calContext: Reference to the calibration context
     * @param surface: Target surface on which the fit result is expressed.
     * @param slType: Switch of which source link type shall be used to access the surfaces from
     *                the measurements & for calibration */
    FitterOptions_t configureFit(const Acts::GeometryContext& tgContext,
                                 const Acts::MagneticFieldContext& mfContext,
                                 const Acts::CalibrationContext& calContext,
                                 const Acts::Surface* surface,
                                 detail::SourceLinkType slType) const;


  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};
  // the settable job options
  Gaudi::Property< double > m_option_outlierChi2Cut {this, "OutlierChi2Cut", 12.5, 
      "Chi2 cut used by the outlier finder" };
  Gaudi::Property< double > m_option_ReverseFilteringPt {this, "ReverseFilteringPt", 1.0 * Acts::UnitConstants::GeV,
      "Pt cut used for the ReverseFiltering logic"};
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};
  Gaudi::Property< double > m_option_seedCovarianceScale {this, "SeedCovarianceScale", 100.,
      "Scale factor for the input seed covariance when doing refitting"};

  /** @brief Calibrator for the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasurementCalibrator m_trkCalibrator{};
  /** @brief Accessor to fetch surfaces from the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasSurfaceAccessor m_trkSurfAcc{};
  /** @brief Calibrator for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataCalibrator m_prdCalibrator{};
  /** @brief Surface accessor for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataSurfaceAcc m_prdSurfAcc{};
  /** @brief Accessor to fetch surfaces from the xAOD::UncalibratedMeasurements (Phase-II EDM) */
  detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc{};
  /** @brief Calibrator of the uncalibrated measurements */
  using xAODUnCalibrator_t = detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend> ;
  xAODUnCalibrator_t m_uncalibMeasCalibrator{};


  /// Type erased track fitter function.
    using Fitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::Navigator>, ActsTrk::MutableTrackStateBackend>;
    std::unique_ptr<Fitter> m_fitter {nullptr};

    using DirectFitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::DirectNavigator>, ActsTrk::MutableTrackStateBackend>;
    std::unique_ptr<DirectFitter> m_directFitter {nullptr};

      /** @brief Array of all configured fitter extensions depending on which source link type is in use */
    static constexpr unsigned s_nExtensions = static_cast<unsigned>(detail::SourceLinkType::nTypes);
    std::array<FitterExtension_t, s_nExtensions>  m_kfExtensions{};

    ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};
    ActsTrk::detail::FitterHelperFunctions::ReverseFilteringLogic m_reverseFilteringLogic{0};

  /// Private access to the logger
  const Acts::Logger& logger() const {
    return *m_logger;
  }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator {this, "RotCreatorTool", ""};
  //Gaudi Property to choose from PRD or ROT measurment ReFit
  Gaudi::Property<bool> m_doReFitFromPRD{this, "DoReFitFromPRD", false, "Do Refit From PRD instead of ROT"};
}; // end of namespace

}
#endif

