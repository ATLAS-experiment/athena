/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingGNNAlg.h"

// Athena
#include "AthenaKernel/Chrono.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"

// ActsTrk
#include "ActsEvent/TrackContainerUtils.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

// STL
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace Acts::UnitLiterals;

namespace ActsTrk {


TrackFindingGNNAlg::TrackFindingGNNAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

TrackFindingGNNAlg::~TrackFindingGNNAlg() = default;

// === initialize ==========================================================

StatusCode TrackFindingGNNAlg::initialize() {
  // Athena tools
  m_logger = makeActsAthenaLogger(this, "Acts GNN Algorithm");
  ACTS_DEBUG("TrackFindingGNNAlg::initialize() - begin");
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_ctxProvider.initialize());
  ATH_CHECK(m_xaodPixelSpacePointContainerKey.initialize());
  ATH_CHECK(m_xaodStripSpacePointContainerKey.initialize());
  ATH_CHECK(m_xaodStripSpacePointOverlapContainerKey.initialize());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_chronoSvc.retrieve());
  ATH_CHECK(m_paramEstimationTool.retrieve());
  ATH_CHECK(m_fitterTool.retrieve());
  ATH_CHECK(m_gnnPipelineTool.retrieve());

  m_uncalibMeasSurfAccessor =
      detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()};
  m_uncalibMeasCalibrator =
      detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend>::
          NoCalibration(m_trackingGeometrySvc.get());

  using TSC = Acts::TrackSelector::Config;
  m_trackSelectorConfig = Acts::TrackSelector::EtaBinnedConfig(0.0);

  auto commonConfig = [&](TSC &config) {
    config.requireReferenceSurface = true;
    config.loc1Min = m_offlineZ0Sel.value() ? -200_mm : -150_mm; // z0 min
    config.loc1Max = m_offlineZ0Sel.value() ? 200_mm : 150_mm;   // z0 max
  };

  m_trackSelectorConfig
      .addCuts(2.0,
               [&](TSC &config) {
                 commonConfig(config);
                 config.maxHoles = m_relaxCentralHoleSel.value() ? 4 : 2; 
                 config.minMeasurements = m_relaxMeasurementSel.value() ? 7 : 9; 
                 config.ptMin = 900_MeV;
                 config.loc0Max = 2_mm;    // d0 max
                 config.loc0Min = -2_mm;   // d0 min
               })
      .addCuts(2.6,
               [&](TSC &config) {
                 commonConfig(config);
                 config.maxHoles = m_relaxCentralHoleSel.value() ? 4 : 2;
                 config.minMeasurements = m_relaxMeasurementSel.value() ? 7 : 8;
                 config.ptMin = 400_MeV;
                 config.loc0Max = 2_mm;    // d0 max
                 config.loc0Min = -2_mm;   // d0 min
               })
      .addCuts([&](TSC &config) {
        commonConfig(config);
        config.maxHoles = 2;
        config.minMeasurements = 7;
        config.ptMin = 400_MeV;
        config.loc0Max = 10_mm;   // d0 max
        config.loc0Min = -10_mm;  // d0 min
      });

  ACTS_INFO("Track selector config:\n" << m_trackSelectorConfig);

  ACTS_DEBUG("TrackFindingGNNAlg::initialize() - end");
  return StatusCode::SUCCESS;
}

// === execute =============================================================

StatusCode TrackFindingGNNAlg::execute(const EventContext &ctx) const {
  ACTS_DEBUG("TrackFindingGNNAlg::execute() - begin");

  std::optional<Athena::Chrono> timer;
  timer.emplace("GNN get spacepoint handles", m_chronoSvc.get());

  const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);
  const Acts::MagneticFieldContext mctx = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::CalibrationContext cctx = m_ctxProvider.getCalibrationContext(ctx);

  auto detElToGeoIdMap = m_trackingGeometrySvc->surfaceIdMap();

  // Collect the spacepoint containers and hand them to the GNN pipeline
  auto pixelSPHandle = SG::makeHandle(m_xaodPixelSpacePointContainerKey, ctx);
  ATH_CHECK(pixelSPHandle.isValid());
  auto stripSPHandle = SG::makeHandle(m_xaodStripSpacePointContainerKey, ctx);
  ATH_CHECK(stripSPHandle.isValid());
  auto stripSPOVHandle =
      SG::makeHandle(m_xaodStripSpacePointOverlapContainerKey, ctx);
  ATH_CHECK(stripSPOVHandle.isValid());

  std::vector<const xAOD::SpacePointContainer*> spacePointCollections{
      pixelSPHandle.cptr(), stripSPHandle.cptr(), stripSPOVHandle.cptr()};

  timer.reset();
  timer.emplace("GNN seed building", m_chronoSvc.get());

  ActsTrk::SeedContainer gnnSeeds;
  ATH_CHECK(m_gnnPipelineTool->buildSeed(spacePointCollections, gnnSeeds));

  ACTS_DEBUG("GNN produced " << gnnSeeds.size() << " seed candidates");

  timer.reset();
  timer.emplace("GNN parameter estimation + fit", m_chronoSvc.get());

  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  constexpr std::size_t nTracksExpected = 3000;
  trackBackend.reserve(nTracksExpected);
  trackStateBackend.reserve(nTracksExpected * 30);
  detail::RecoTrackContainer tracks(trackBackend, trackStateBackend);
  TrackContainerUtils::addFitterTypeProperty(tracks);

  auto retrieveSurface = [&](const ActsTrk::Seed& seed, bool useTopSp) -> const Acts::Surface& {
    const xAOD::SpacePoint* sp = useTopSp ? seed.sp().front() : seed.sp().back();
    auto geoId = ActsTrk::getSurfaceGeometryIdOfMeasurement(*detElToGeoIdMap, *sp->measurements().front());
    const auto* surface = m_trackingGeometrySvc->trackingGeometry()->findSurface(geoId);
    if (!surface) {
      throw std::runtime_error("retrieveSurface: no Acts surface for GeometryIdentifier " + std::to_string(geoId.value()));
    }
    return *surface;
  };

  for (const ActsTrk::Seed gnnSeed : gnnSeeds) {
    ActsTrk::SpacePointRange cand = gnnSeed.sp();

    // The parameter estimation tool selects the SPs used for the estimate (see its parameterEstimationMode property)
    const auto& [initialParamsOpt, estimationStatus] = m_paramEstimationTool->estimateTrackParameters(
        gnnSeed, /*useTopSp=*/true, gctx, mctx, cctx, retrieveSurface);
    if (!initialParamsOpt.has_value()) continue;

    // Space points are already ordered by radius by the GNN pipeline tool
    std::vector<const xAOD::UncalibratedMeasurement*> measList;
    measList.reserve(cand.size() * 2);
    for (const xAOD::SpacePoint* sp : cand) {
      for (const xAOD::UncalibratedMeasurement* m : sp->measurements()) {
        measList.push_back(m);
      }
    }

    auto fitted = m_fitterTool->fit(measList, *initialParamsOpt, gctx, mctx, cctx);
    if (fitted) {
      for (auto track : *fitted) {
        auto newTrack = tracks.makeTrack();
        newTrack.copyFrom(track);
      }
    }
  }

  ACTS_DEBUG("After track fit: " << tracks.size() << " / " << gnnSeeds.size()
                                 << " successfull");

  timer.reset();
  timer.emplace("Track selection & conversion", m_chronoSvc.get());

  Acts::VectorTrackContainer selTrackBackend;
  selTrackBackend.reserve(trackBackend.size());
  detail::RecoTrackContainer selectedTracks(selTrackBackend, trackStateBackend);
  TrackContainerUtils::addFitterTypeProperty(selectedTracks);

  Acts::TrackSelector selector(m_trackSelectorConfig);
  for (auto track : tracks) {
    if (selector.isValidTrack(track)) {
      auto newTrack = selectedTracks.makeTrack();
      
      // v45: copyFrom now copies everything including tip/stem indices
      newTrack.copyFrom(track);
    }
  }

  ACTS_DEBUG("GNN seeds: " << gnnSeeds.size() << ", fitted: " << tracks.size()
                           << ", selected: " << selectedTracks.size());

  // Write tracks to storage again
  Acts::ConstVectorTrackContainer constTrackBackend(std::move(selTrackBackend));
  Acts::ConstVectorMultiTrajectory constTrackStateBackend(std::move(trackStateBackend));
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer
    = std::make_unique<ActsTrk::TrackContainer>(std::move(constTrackBackend), std::move(constTrackStateBackend) );
   
  ACTS_DEBUG("Storing track collection with key '" << m_trackContainerKey.key() << "'");
  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  return StatusCode::SUCCESS;
}
} // namespace ActsTrk
