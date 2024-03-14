/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GAUSSIANSUMFITTER_H
#define ACTSGEOMETRY_GAUSSIANSUMFITTER_H



// ATHENA 
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"

// ACTS
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/TrackFitting/BetheHeitlerApprox.hpp"
#include "Acts/TrackFitting/GsfOptions.hpp"
#include "Acts/Utilities/Logger.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "FitterHelperFunctions.h"
#include "MeasurementCalibrator.h"
#include "ActsGeometry/ATLASSourceLinkSurfaceAccessor.h"

// STL
#include <string>
#include <memory>

namespace ActsTrk {

class GaussianSumFitter : public extends<AthAlgTool, Trk::ITrackFitter> {
public:
  
  GaussianSumFitter(const std::string&, const std::string&, const IInterface*);
  virtual ~GaussianSumFitter() = default;

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


  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:
  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend> prepareOptions(const Acts::GeometryContext& tgContext,
									    const Acts::MagneticFieldContext& mfContext,
									    const Acts::CalibrationContext& calContext,
									    const Acts::PerigeeSurface& surface) const;
  
  std::unique_ptr<Trk::Track> performFit(const EventContext& ctx,
					 const Acts::GeometryContext& tgContext,
					 const Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>& gsfOptions,
					 const std::vector<Acts::SourceLink>& trackSourceLinks,
					 const Acts::BoundTrackParameters& initialParams) const;

  // Create a track from the fitter result
  std::unique_ptr<Trk::Track> makeTrack(const EventContext& ctx, 
          const Acts::GeometryContext& tgContext, 
          ActsTrk::MutableTrackContainer& tracks,
          Acts::Result<typename ActsTrk::MutableTrackContainer::TrackProxy, std::error_code>& fitResult) const;

  const Acts::GsfExtensions<ActsTrk::MutableTrackStateBackend>& getExtensions() const;

  /// Private access to the logger
  const Acts::Logger& logger() const;

 private:
  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};
  ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool {this, "SummaryTool", "", "ToolHandle for track summary tool"};
  ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool {this, 
                                                           "BoundaryCheckTool", 
                                                           "",
                                                           "Boundary checking tool for detector sensitivities"};
  
    // the settable job options
  Gaudi::Property< double > m_option_outlierChi2Cut {this, "OutlierChi2Cut", 12.5, 
      "Chi2 cut used by the outlier finder" };
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};

  Gaudi::Property<bool> m_refitOnly{this, "RefitOnly", false, "Do refit only. Track summary will not be added"};

  /// Type erased track fitter function.
  using Fitter = Acts::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator>,
                                                        Acts::AtlasBetheHeitlerApprox<6, 5>,
                                                        ActsTrk::MutableTrackStateBackend>;

  std::unique_ptr<TrkMeasurementCalibrator> m_calibrator;
  std::unique_ptr<Fitter> m_fitter;


  ATLASSourceLinkSurfaceAccessor m_surfaceAccessor{};
  Acts::GsfExtensions<ActsTrk::MutableTrackStateBackend> m_gsfExtensions;

  ActsTrk::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

}; // end of namespace

}

#endif

