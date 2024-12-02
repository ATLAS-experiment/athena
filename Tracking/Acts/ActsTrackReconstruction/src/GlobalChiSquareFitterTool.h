/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H
#define ACTSGEOMETRY_GLOBALCHISQUAREFITTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
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
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "src/detail/TrkMeasurementCalibrator.h"

// STL
#include <cmath>   //std::abs
#include <limits>  //for numeric_limits
#include <memory>  //unique_ptr
#include <string>

#include "ActsToolInterfaces/IFitterTool.h"

class EventContext;

namespace Trk {
class Track;
class PrepRawData;
}  // namespace Trk

namespace ActsTrk {

// TODO: those are defined in KalmanFitter.h -> make more general?
struct PRDSourceLinkGX2F {
  const Trk::PrepRawData* prd{nullptr};
};

struct PRDSourceLinkCalibratorGX2F {
  template <typename trajectory_t>
  void calibrate(const Acts::GeometryContext& gctx,
                 const Acts::CalibrationContext& cctx,
                 const Acts::SourceLink& sl,
                 typename trajectory_t::TrackStateProxy trackState) const;

  const Trk::IRIO_OnTrackCreator* rotCreator{nullptr};
  const Trk::IRIO_OnTrackCreator* broadRotCreator{nullptr};
  const ActsTrk::IActsToTrkConverterTool* converterTool{nullptr};
};

struct PRDSourceLinkSurfaceAccessorGX2F {
  const ActsTrk::IActsToTrkConverterTool* converterTool{nullptr};

  const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
};

class GlobalChiSquareFitterTool
    : public extends<AthAlgTool, Trk::ITrackFitter, ActsTrk::IFitterTool> {
 public:
  GlobalChiSquareFitterTool(const std::string&, const std::string&,
                            const IInterface*);
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
  virtual std::unique_ptr<ActsTrk::MutableTrackContainer> fit(
      const EventContext& ctx,
      const std::vector<ActsTrk::ATLASUncalibSourceLink>& clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const DetectorElementToActsGeometryIdMap& detectorElementToGeometryIdMap,
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
  virtual std::unique_ptr<ActsTrk::MutableTrackContainer> fit(
      const EventContext& ctx, const ActsTrk::Seed& seed,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const DetectorElementToActsGeometryIdMap& detectorElementToGeometryIdMap)
      const override;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
 private:
  // Create a track from the fitter result
  std::unique_ptr<Trk::Track> makeTrack(
      const EventContext& ctx, Acts::GeometryContext& tgContext,
      ActsTrk::MutableTrackContainer& tracks,
      Acts::Result<ActsTrk::MutableTrackContainer::TrackProxy, std::error_code>&
          fitResult,
      bool SourceLinkType = false) const;
  // parameter (bool) SourceLinkType to distinguish between ATLASSourceLink and
  // PRDSourceLink implementation. bool SourceLinkType = false for
  // ATLASSourceLink bool SourceLinkType = true for PRDSourceLink

  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{
      this, "ExtrapolationTool", ""};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};
  ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{
      this, "ATLASConverterTool", ""};
  ToolHandle<Trk::IExtendedTrackSummaryTool> m_trkSummaryTool{
      this, "SummaryTool", "", "ToolHandle for track summary tool"};
  ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool{
      this, "BoundaryCheckTool", "",
      "Boundary checking tool for detector sensitivities"};

  // the settable job options
  Gaudi::Property<double> m_option_outlierChi2Cut{
      this, "OutlierChi2Cut", 12.5, "Chi2 cut used by the outlier finder"};
  Gaudi::Property<int> m_option_maxPropagationStep{
      this, "MaxPropagationStep", 5000,
      "Maximum number of steps for one propagate call"};
  Gaudi::Property<double> m_option_seedCovarianceScale{
      this, "SeedCovarianceScale", 100.,
      "Scale factor for the input seed covariance when doing refitting"};

  std::unique_ptr<ActsTrk::detail::TrkMeasurementCalibrator> m_calibrator{
      nullptr};

  /// Type erased track fitter function.
  using Fitter = Acts::Experimental::Gx2Fitter<
      Acts::Propagator<Acts::SympyStepper, Acts::Navigator>,
      ActsTrk::MutableTrackStateBackend>;
  std::unique_ptr<Fitter> m_fitter{nullptr};

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      m_gx2fExtensions;

  ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// Private access to the logger
  const Acts::Logger& logger() const { return *m_logger; }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  ToolHandle<Trk::IRIO_OnTrackCreator> m_broadROTcreator{
      this, "BroadRotCreatorTool", ""};
  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator{this, "RotCreatorTool", ""};
  // Gaudi Property to choose from PRD or ROT measurement ReFit
  Gaudi::Property<bool> m_doReFitFromPRD{this, "DoReFitFromPRD", false,
                                         "Do Refit From PRD instead of ROT"};
};

}  // namespace ActsTrk
#endif
