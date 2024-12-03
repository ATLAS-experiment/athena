/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ScoreBasedAmbiguityResolutionAlg.h"

#include "ScoreBasedSolverCutsImpl.h"

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

namespace {
std::size_t sourceLinkHash(const Acts::SourceLink &slink) {
  const ActsTrk::ATLASUncalibSourceLink &atlasSourceLink =
      slink.get<ActsTrk::ATLASUncalibSourceLink>();
  const xAOD::UncalibratedMeasurement &uncalibMeas =
      ActsTrk::getUncalibratedMeasurement(atlasSourceLink);
  return uncalibMeas.identifier();
}

bool sourceLinkEquality(const Acts::SourceLink &a, const Acts::SourceLink &b) {
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
    cfg.pTMin = m_pTMin;
    cfg.pTMax = m_pTMax;
    cfg.phiMin = m_phiMin;
    cfg.phiMax = m_phiMax;
    cfg.etaMin = m_etaMin;
    cfg.etaMax = m_etaMax;
    cfg.useAmbiguityFunction = m_useAmbiguityFunction;

    m_ambi = std::make_unique<Acts::ScoreBasedAmbiguityResolution>(
        std::move(cfg), makeActsAthenaLogger(this, "Acts"));
    assert(m_ambi);
  }

  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_tracksKey.initialize());
  ATH_CHECK(m_resolvedTracksKey.initialize());
  ATH_CHECK(m_resolvedTracksBackendHandles.initialize(
      ActsTrk::prefixFromTrackContainerName(
          m_resolvedTracksKey.key())));  // TODO choose prefix related to the
                                         // output tracks name
  return StatusCode::SUCCESS;
}

StatusCode ScoreBasedAmbiguityResolutionAlg::execute(
    const EventContext &ctx) const {
  auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
  auto mon = Monitored::Group(m_monTool, timer);

  SG::ReadHandle<ActsTrk::TrackContainer> trackHandle =
      SG::makeHandle(m_tracksKey, ctx);
  ATH_CHECK(trackHandle.isValid());

  // creates mutable tracks from the input tracks to add summary information
  // NOTE: this operation likely needs to moved outside ambiguity resolution
  ActsTrk::MutableTrackContainer updatedTracks =
      ScoreBasedSolverCutsImpl::addSummaryInformation(*trackHandle);

  // create the optional cuts for the ambiguity resolution
  Acts::ScoreBasedAmbiguityResolution::OptionalCuts<
      ActsTrk::MutableTrackContainer::ConstTrackProxy>
      optionalCuts;

  using TrackProxyType = Acts::TrackProxy<ActsTrk::MutableTrackSummaryContainer,
                                          ActsTrk::MutableMultiTrajectory,
                                          Acts::detail::ValueHolder, true>;

  // Eta based optional cuts is added as a lambda function inorder to access the
  // m_etaDependentCutsSvc private variable
  optionalCuts.cuts.push_back([this](const TrackProxyType &track) {
    // Access m_etaDependentCutsSvc through this
    return ScoreBasedSolverCutsImpl::etaDependentCuts(
        track, this->m_etaDependentCutsSvc);
  });

  // Add other optional cuts and scores
  optionalCuts.cuts.push_back(ScoreBasedSolverCutsImpl::doubleHolesFilter);
  optionalCuts.scores.push_back(
      ScoreBasedSolverCutsImpl::innermostPixelLayerHitsScore);
  optionalCuts.scores.push_back(
      ScoreBasedSolverCutsImpl::ContribPixelLayersScore);
  optionalCuts.hitSelections.push_back(
      ScoreBasedSolverCutsImpl::patternTrackHitSelection);

  // Call the ambiguity resolution algorithm with the optional cuts on the
  // updated tracks
  std::vector<int> goodTracks = m_ambi->solveAmbiguity(
      updatedTracks, &sourceLinkHash, &sourceLinkEquality, optionalCuts);

  ATH_MSG_DEBUG("Resolved to " << goodTracks.size() << " tracks from "
                               << updatedTracks.size());

  ActsTrk::MutableTrackContainer solvedTracks;
  solvedTracks.ensureDynamicColumns(updatedTracks);

  for (auto iTrack : goodTracks) {
    auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
    destProxy.copyFrom(updatedTracks.getTrack(iTrack));
  }
  std::unique_ptr<ActsTrk::TrackContainer> outputTracks =
      m_resolvedTracksBackendHandles.moveToConst(
          std::move(solvedTracks),
          m_trackingGeometryTool->getGeometryContext(ctx).context(), ctx);
  SG::WriteHandle<ActsTrk::TrackContainer> resolvedTrackHandle(
      m_resolvedTracksKey, ctx);

  if (resolvedTrackHandle.record(std::move(outputTracks)).isFailure()) {
    ATH_MSG_ERROR("Failed to record resolved ACTS tracks with key "
                  << m_resolvedTracksKey.key());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
