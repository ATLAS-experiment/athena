/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_GAUSSIANSUMFITTERTOOL_H
#define ACTSTRACKRECONSTRUCTION_GAUSSIANSUMFITTERTOOL_H

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
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "src/detail/FitterHelperFunctions.h"

#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsCalibrators/TrkMeasSurfaceAccessor.h"

// STL
#include <string>
#include <memory>



namespace ActsTrk {

class GaussianSumFitterTool
  : public extends<AthAlgTool, Trk::ITrackFitter, ActsTrk::IFitterTool> {
public:
  
  GaussianSumFitterTool(const std::string&, const std::string&, const IInterface*);
  virtual ~GaussianSumFitterTool() = default;

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

  //! Acts seed fit
  virtual
    std::unique_ptr< ActsTrk::MutableTrackContainer >
    fit(const ActsTrk::Seed &seed,
        const Acts::BoundTrackParameters& initialParams,
        const Acts::GeometryContext& tgContext,
        const Acts::MagneticFieldContext& mfContext,
        const Acts::CalibrationContext& calContext) const override;

  virtual
  std::unique_ptr< ActsTrk::MutableTrackContainer >
  fit(const std::vector< ActsTrk::ATLASUncalibSourceLink> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,      
      const Acts::Surface* targetSurface) const override;
  
  virtual StatusCode fit(
    const EventContext& ctx,
        const ActsTrk::TrackContainer::ConstTrackProxy& track,          
    ActsTrk::MutableTrackContainer& trackContainer) const override;

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

  std::unique_ptr<Trk::Track> performDirectFit(const EventContext& ctx,
                 const Acts::GeometryContext& tgContext,
                 const Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>& gsfOptions,
                 const std::vector<Acts::SourceLink>& trackSourceLinks,
                 const Acts::BoundTrackParameters& initialParams,
                 const std::vector<const Acts::Surface*>& surfaces) const;

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
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
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

  Gaudi::Property< int > m_maxComponents {this, "MaxComponents", 12,
      "Maximum number of components in GSF"};

  Gaudi::Property<bool> m_useDirectNavigation{this, "UseDirectNavigation", false,
                "GSF with direct navigation when refitting measurements"};

  Gaudi::Property<bool> m_refitOnly{this, "RefitOnly", false,
            "Do refit only. Track summary will not be added"};

  Gaudi::Property< double > m_weightCutOff {this, "WeightCutOff", 1.e-4,
              "component weight cut off"};

  Gaudi::Property<std::string> m_option_componentMergeMethod{this, "ComponentMergeMethod", "MaxWeight"
                  , "method to merge components {Mean, MaxWeight}"};

  Acts::ComponentMergeMethod m_componentMergeMethod;

  /// Type erased track fitter function.
  using Fitter = Acts::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator>,
                                                        Acts::AtlasBetheHeitlerApprox<6, 5>,
                                                        ActsTrk::MutableTrackStateBackend >;

  std::unique_ptr<ActsTrk::detail::TrkMeasurementCalibrator> m_calibrator {nullptr};
  std::unique_ptr<Fitter> m_fitter {nullptr};

  using DirectFitter = Acts::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::DirectNavigator>,
            Acts::AtlasBetheHeitlerApprox<6, 5>,
            ActsTrk::MutableTrackStateBackend >;
  std::unique_ptr<DirectFitter> m_directFitter {nullptr};


  detail::TrkMeasSurfaceAccessor m_surfaceAccessor{};
  Acts::GsfExtensions<ActsTrk::MutableTrackStateBackend> m_gsfExtensions;

  ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger {nullptr};

}; // end of namespace

}

#endif

