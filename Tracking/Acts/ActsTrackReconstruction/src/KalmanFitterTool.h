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

#include "ActsCalibration/TrkMeasurementCalibrator.h"

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
      fit(const EventContext& ctx,
	    const std::vector<ActsTrk::ATLASUncalibSourceLink> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap,
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
    fit(const EventContext& ctx,
	const ActsTrk::Seed &seed,
	const Acts::BoundTrackParameters& initialParams,
	const Acts::GeometryContext& tgContext,
	const Acts::MagneticFieldContext& mfContext,
	const Acts::CalibrationContext& calContext,
	const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap) const override;


    virtual StatusCode fit(
        const EventContext& ctx,
                const ActsTrk::TrackContainer::ConstTrackProxy& track,          
        ActsTrk::MutableTrackContainer& trackContainer) const override;
  
  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

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

  std::unique_ptr<ActsTrk::detail::TrkMeasurementCalibrator> m_calibrator {nullptr};

  /// Type erased track fitter function.
    using Fitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::Navigator>, ActsTrk::MutableTrackStateBackend>;
    std::unique_ptr<Fitter> m_fitter {nullptr};

    using DirectFitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::DirectNavigator>, ActsTrk::MutableTrackStateBackend>;
    std::unique_ptr<DirectFitter> m_directFitter {nullptr};

    Acts::KalmanFitterExtensions<ActsTrk::MutableTrackStateBackend> m_kfExtensions;

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

