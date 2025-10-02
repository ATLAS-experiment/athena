/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ScoreBasedAmbiguityResolutionAlg.h"

// Athena
#include "AthenaMonitoringKernel/Monitored.h"
#include "PathResolver/PathResolver.h"

// ACTS
#include <fstream>
#include <iostream>
#include <vector>

#include "Acts/AmbiguityResolution/ScoreBasedAmbiguityResolution.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Plugins/Json/AmbiguityConfigJsonConverter.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsInterop/Logger.h"
#include "ActsInterop/TableUtils.h"
#include "src/detail/MeasurementIndex.h"
#include "src/detail/SharedHitCounter.h"
#include "src/detail/Definitions.h"

namespace {
static std::size_t sourceLinkHash(const Acts::SourceLink &slink) {
  const ActsTrk::ATLASUncalibSourceLink &atlasSourceLink =
      slink.get<ActsTrk::ATLASUncalibSourceLink>();
  const xAOD::UncalibratedMeasurement &uncalibMeas =
      ActsTrk::getUncalibratedMeasurement(atlasSourceLink);
  return uncalibMeas.identifier();
}

static bool sourceLinkEquality(const Acts::SourceLink &a, const Acts::SourceLink &b) {
  const xAOD::UncalibratedMeasurement &uncalibMeas_a =
      ActsTrk::getUncalibratedMeasurement(
          a.get<ActsTrk::ATLASUncalibSourceLink>());
  const xAOD::UncalibratedMeasurement &uncalibMeas_b =
      ActsTrk::getUncalibratedMeasurement(
          b.get<ActsTrk::ATLASUncalibSourceLink>());

  return uncalibMeas_a.identifier() == uncalibMeas_b.identifier();
}

}  // namespace

namespace ActsTrk {

ScoreBasedAmbiguityResolutionAlg::ScoreBasedAmbiguityResolutionAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode ScoreBasedAmbiguityResolutionAlg::initialize() {
  {
    Acts::ScoreBasedAmbiguityResolution::Config cfg;
    Acts::ConfigPair configPair;
    nlohmann::json json_file;

    std::string fileName = PathResolver::find_file(
        "ActsConfig/ActsAmbiguityConfig.json", "DATAPATH");

    std::ifstream file(fileName.c_str());
    if (!file.is_open()) {
      std::cerr << "Error opening file: " << fileName << std::endl;
      return {};
    }
    file >> json_file;
    file.close();

    Acts::from_json(json_file, configPair);

    cfg.volumeMap = configPair.first;
    cfg.detectorConfigs = configPair.second;
    cfg.minScore = m_minScore;
    cfg.minScoreSharedTracks = m_minScoreSharedTracks;
    cfg.maxSharedTracksPerMeasurement = m_maxSharedTracksPerMeasurement;
    cfg.maxShared = m_maxShared;
    cfg.minUnshared = m_minUnshared;
    cfg.useAmbiguityScoring = m_useAmbiguityScoring;

    m_ambi = std::make_unique<Acts::ScoreBasedAmbiguityResolution>(
        std::move(cfg), makeActsAthenaLogger(this, "Acts"));
    assert(m_ambi);
  }

  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
  ATH_CHECK(m_tracksKey.initialize());
  ATH_CHECK(m_resolvedTracksKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ScoreBasedAmbiguityResolutionAlg::finalize() {
  ATH_MSG_INFO("Score-based Ambiguity Resolution statistics" << std::endl
               << makeTable(m_stat,
                            std::array<std::string, kNStat>{
                                "Input tracks",
                                "Resolved tracks",
                                "Total shared hits"}).columnWidth(10));
  return StatusCode::SUCCESS;
}

StatusCode ScoreBasedAmbiguityResolutionAlg::execute(
    const EventContext &ctx) const {
  auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
  auto mon = Monitored::Group(m_monTool, timer);

  SG::ReadHandle<ActsTrk::TrackContainer> trackHandle =
      SG::makeHandle(m_tracksKey, ctx);
  ATH_CHECK(trackHandle.isValid());
  const ActsTrk::TrackContainer* trackContainer = trackHandle.cptr();
  m_stat[kNInputTracks] += trackContainer->size();

  // creates mutable tracks from the input tracks to add summary information
  // NOTE: this operation likely needs to moved outside ambiguity resolution
  auto updatedTracks =
    ScoreBasedSolverCutsImpl::addSummaryInformation(*trackContainer);
  
  // create the optional cuts for the ambiguity resolution
  Acts::ScoreBasedAmbiguityResolution::Optionals<
    typename decltype(updatedTracks)::ConstTrackProxy>
      Optionals;

  // Adding optional cuts
  Optionals.cuts.push_back(ScoreBasedSolverCutsImpl::etaDependentCuts);

  // Adding optional Score Modifiers
  Optionals.scores.push_back(ScoreBasedSolverCutsImpl::doubleHolesScore);
  Optionals.scores.push_back(ScoreBasedSolverCutsImpl::nSCTPixelHitsScore);
  Optionals.scores.push_back(
      ScoreBasedSolverCutsImpl::innermostPixelLayerHitsScore);
  Optionals.scores.push_back(ScoreBasedSolverCutsImpl::ContribPixelLayersScore);

  // Call the ambiguity resolution algorithm with the optional cuts on the
  // updated tracks
  std::vector<int> goodTracks = m_ambi->solveAmbiguity(
      updatedTracks, &sourceLinkHash, &sourceLinkEquality, Optionals);

  ATH_MSG_DEBUG("Resolved to " << goodTracks.size() << " tracks from "
                << updatedTracks.size());
  m_stat[kNResolvedTracks] += goodTracks.size();


  // we start shortlisting the container
  Acts::VectorTrackContainer resolvedTrackBackend;
  Acts::VectorMultiTrajectory resolvedTrackStateBackend;
  detail::RecoTrackContainer resolvedTracksContainer(resolvedTrackBackend, resolvedTrackStateBackend);
  
  resolvedTracksContainer.ensureDynamicColumns(updatedTracks);
  
  detail::MeasurementIndex measurementIndex;
  detail::SharedHitCounter sharedHits;

  std::size_t totalShared = 0;
  for (auto iTrack : goodTracks) {
    auto destProxy = resolvedTracksContainer.getTrack(resolvedTracksContainer.addTrack());
    destProxy.copyFrom(updatedTracks.getTrack(iTrack));
    
    if (m_countSharedHits) {
      auto [nShared, nBadTrackMeasurements] = sharedHits.computeSharedHitsDynamic(destProxy, resolvedTracksContainer, measurementIndex);
      if (nBadTrackMeasurements > 0) {
        ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input track");
      }
      totalShared += nShared;
    }
  }
  
  if (m_countSharedHits) {
    ATH_MSG_DEBUG("total number of shared hits = " << totalShared);
    m_stat[kNSharedHits] += totalShared;
  }

  // make const collection
  Acts::ConstVectorTrackContainer storableTrackBackend( std::move(resolvedTrackBackend) );
  Acts::ConstVectorMultiTrajectory storableTrackStateBackend( std::move(resolvedTrackStateBackend) );
  std::unique_ptr< ActsTrk::TrackContainer > storableTracksContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(storableTrackBackend),
                                                                                                                    std::move(storableTrackStateBackend) );
  SG::WriteHandle<ActsTrk::TrackContainer> resolvedTrackHandle = SG::makeHandle( m_resolvedTracksKey, ctx );
  if (resolvedTrackHandle.record( std::move(storableTracksContainer)).isFailure()) {
    ATH_MSG_ERROR("Failed to record resolved ACTS tracks with key " << m_resolvedTracksKey.key() );
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
