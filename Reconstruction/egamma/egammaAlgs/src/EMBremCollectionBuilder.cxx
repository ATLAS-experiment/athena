/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EMBremCollectionBuilder.h"

#include "AthenaKernel/errorcheck.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/TrackSummary.h"

#include "egammaUtils/egammaCopyTrackParticleInfo.h"

#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <algorithm>
#include <memory>


EMBremCollectionBuilder::EMBremCollectionBuilder(const std::string& name,
                                                 ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
EMBremCollectionBuilder::initialize()
{

  ATH_CHECK(m_selectedTrackParticleContainerKey.initialize());
  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_OutputTrkPartContainerKey.initialize());
  ATH_CHECK(m_OutputTrackContainerKey.initialize());

  /* retrieve the track refitter tool*/
  ATH_CHECK(m_trkRefitTool.retrieve());
  /* Get the particle creation tool */
  ATH_CHECK(m_particleCreatorTool.retrieve());
  /* Get the track slimming tool if needed */
  if (m_doSlimTrkTracks) {
    ATH_CHECK(m_slimTool.retrieve());
  } else {
    m_slimTool.disable();
  }

  return StatusCode::SUCCESS;
}

StatusCode
EMBremCollectionBuilder::EMBremCollectionBuilder::finalize()
{
  ATH_MSG_INFO("====> GSF fitting Statistics ============");
  ATH_MSG_INFO("RefittedTracks: " << m_RefittedTracks);
  ATH_MSG_INFO("Failed Fit Tracks: " << m_FailedFitTracks);
  ATH_MSG_INFO("Not refitted due to selection: " << m_FailedSiliconRequirFit);
  ATH_MSG_INFO("<========================================");
  return StatusCode::SUCCESS;
}

StatusCode
EMBremCollectionBuilder::execute(const EventContext& ctx) const
{
  // Read the inputs
  // The input track particles
  SG::ReadHandle<xAOD::TrackParticleContainer> trackTES(
    m_trackParticleContainerKey, ctx);
  ATH_CHECK(trackTES.isValid());

  // The input selected track particles (subset of "all" input
  // read above).
  SG::ReadHandle<xAOD::TrackParticleContainer> selectedTrackParticles(
    m_selectedTrackParticleContainerKey, ctx);
  ATH_CHECK(selectedTrackParticles.isValid());

  // Create the final containers to be written out
  // 1. Track particles
  SG::WriteHandle<xAOD::TrackParticleContainer> finalTrkPartContainer(
    m_OutputTrkPartContainerKey, ctx);
  ATH_CHECK(finalTrkPartContainer.record(
    std::make_unique<xAOD::TrackParticleContainer>(),
    std::make_unique<xAOD::TrackParticleAuxContainer>()));
  xAOD::TrackParticleContainer* cPtrTrkPart = finalTrkPartContainer.ptr();

  // 2. Trk::Tracks
  SG::WriteHandle<TrackCollection> finalTracks(m_OutputTrackContainerKey, ctx);
  ATH_CHECK(finalTracks.record(std::make_unique<TrackCollection>()));
  TrackCollection* cPtrTracks = finalTracks.ptr();

  // Loop over the selected track-particles
  // Split TRT-alone from silicon ones.
  // For the TRT we can get all the info
  // already since we are not going to refit.
  std::vector<const xAOD::TrackParticle*> siliconTrkTracks;
  siliconTrkTracks.reserve(16);
  std::vector<TrackWithIndex> trtAloneTrkTracks;
  trtAloneTrkTracks.reserve(8);

  for (const xAOD::TrackParticle* trackParticle : *selectedTrackParticles) {
    ATH_CHECK(trackParticle->trackLink().isValid());
    const Trk::Track* trktrack = trackParticle->track();
    int nSiliconHits_trk = xAOD::EgammaHelpers::summaryValueInt(
        *trackParticle, xAOD::numberOfSCTHits, 0);
    nSiliconHits_trk += xAOD::EgammaHelpers::summaryValueInt(
        *trackParticle, xAOD::numberOfPixelHits, 0);
    if (nSiliconHits_trk >= m_MinNoSiHits) {
      siliconTrkTracks.push_back(trackParticle);
    } else {
      // copy Trk::Track
      trtAloneTrkTracks.emplace_back(std::make_unique<Trk::Track>(*trktrack),
                                     trackParticle->index());
    }
  }

  // Store the results of the GSF refit
  std::vector<TrackWithIndex> refitted; // refit success
  refitted.reserve(siliconTrkTracks.size());
  std::vector<TrackWithIndex> failedfit; // refit failure

  // Do the GSF refit.
  // Note that altough the input is a xAOD::TrackParticle
  // what we really refit is the corresponding Trk::Track.
  // The output is  two collections
  // one for fit success or failure.
  // TrackWithIndex means the newly created Trk::::Track
  // and the index of the xAOD::TrackParticle  to the
  // original TrackParticle Collection.
 ATH_CHECK(refitTracks(ctx, siliconTrkTracks, refitted, failedfit));

  const size_t refittedCount = refitted.size();
  const size_t failedCount = failedfit.size();
  const size_t trtCount = trtAloneTrkTracks.size();
  const size_t totalCount = refittedCount + failedCount + trtCount;

  // reserve as we know how many we will create
  cPtrTracks->reserve(totalCount);
  cPtrTrkPart->reserve(totalCount);
  // Fill the final collections
  ATH_CHECK(createCollections(ctx,
                              refitted,
                              failedfit,
                              trtAloneTrkTracks,
                              cPtrTracks,
                              cPtrTrkPart,
                              trackTES.ptr()));

  // Update counters
  m_RefittedTracks.fetch_add(refittedCount, std::memory_order_relaxed);
  m_FailedFitTracks.fetch_add(failedCount, std::memory_order_relaxed);
  m_FailedSiliconRequirFit.fetch_add(trtCount, std::memory_order_relaxed);
  return StatusCode::SUCCESS;
}

StatusCode
EMBremCollectionBuilder::refitTracks(
  const EventContext& ctx,
  const std::vector<const xAOD::TrackParticle*>& input,
  std::vector<TrackWithIndex>& refitted,
  std::vector<TrackWithIndex>& failedfit) const
{
  for (const xAOD::TrackParticle* in : input) {
    const Trk::Track* track = in->track();
    IegammaTrkRefitterTool::Cache cache{};
    StatusCode status = m_trkRefitTool->refitTrack(ctx, track, cache);
    if (status == StatusCode::SUCCESS) {
      // this is new track
      refitted.emplace_back(std::move(cache.refittedTrack), in->index());
    } else {
      // This is copy ctor
      failedfit.emplace_back(std::make_unique<Trk::Track>(*track), in->index());
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
EMBremCollectionBuilder::createCollections(
  const EventContext& ctx,
  std::vector<TrackWithIndex>& refitted,
  std::vector<TrackWithIndex>& failedfit,
  std::vector<TrackWithIndex>& trtAlone,
  TrackCollection* finalTracks,
  xAOD::TrackParticleContainer* finalTrkPartContainer,
  const xAOD::TrackParticleContainer* inputTrkPartContainer) const
{
  // Now we can create the final ouput
  // 1. Add the refitted
  for (auto& trk : refitted) {
    ATH_CHECK(createNew(ctx,
                        trk,
                        finalTracks,
                        finalTrkPartContainer,
                        inputTrkPartContainer,
                        true));
  }
  // 2. Add the failed fit
  for (auto& trk : failedfit) {
    ATH_CHECK(createNew(ctx,
                        trk,
                        finalTracks,
                        finalTrkPartContainer,
                        inputTrkPartContainer,
                        false));
  }
  // 3. Add the TRT alone
  for (auto& trk : trtAlone) {
    ATH_CHECK(createNew(ctx,
                        trk,
                        finalTracks,
                        finalTrkPartContainer,
                        inputTrkPartContainer,
                        false));
  }
  return StatusCode::SUCCESS;
}

StatusCode
EMBremCollectionBuilder::createNew(
  const EventContext& ctx,
  TrackWithIndex& trk_info,
  TrackCollection* finalTracks,
  xAOD::TrackParticleContainer* finalTrkPartContainer,
  const xAOD::TrackParticleContainer* inputTrkPartContainer,
  bool isRefitted) const
{

  // Create new TrackParticle owned by finalTrkPartContainer
  xAOD::TrackParticle* aParticle = m_particleCreatorTool->createParticle(
    ctx, *(trk_info.track), finalTrkPartContainer, nullptr, xAOD::electron);

  if (!aParticle) {
    ATH_MSG_WARNING(
      "Could not create TrackParticle!!! for Track: " << *(trk_info.track));
    return StatusCode::SUCCESS;
  }
  size_t origIndex = trk_info.origIndex;
  const xAOD::TrackParticle* original = inputTrkPartContainer->at(origIndex);

  // Add an element link back to original Track Particle collection
  static const SG::AuxElement::Accessor<
    ElementLink<xAOD::TrackParticleContainer>>
    tP("originalTrackParticle");
  ElementLink<xAOD::TrackParticleContainer> linkToOriginal(
    *inputTrkPartContainer, origIndex, ctx);
  tP(*aParticle) = linkToOriginal;
  // Add qoverP from the last measurement
  float QoverPLast(0);
  auto rtsos = trk_info.track->trackStateOnSurfaces()->rbegin();
  for (; rtsos != trk_info.track->trackStateOnSurfaces()->rend(); ++rtsos) {
    if ((*rtsos)->type(Trk::TrackStateOnSurface::Measurement) &&
        (*rtsos)->trackParameters() != nullptr &&
        (*rtsos)->measurementOnTrack() != nullptr &&
        !(*rtsos)->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
      QoverPLast = (*rtsos)->trackParameters()->parameters()[Trk::qOverP];
      break;
    }
  }
  static const SG::AuxElement::Accessor<float> QoverPLM("QoverPLM");
  QoverPLM(*aParticle) = QoverPLast;
  egammaCopyTrackParticleInfo::ToCopy toCopy{.isRefitted = isRefitted,
                                             .doTruth = m_doTruth,
                                             .doPix = m_doPix,
                                             .doSCT = m_doSCT,
                                             .doTRT = m_doTRT,
                                             .doHGTD = m_doHGTD};
  egammaCopyTrackParticleInfo::copy(*aParticle, *original, toCopy);
  // Slim the Trk::Track, store to the new
  // Trk::Track collection and make the Track
  // Particle point to it
  if (m_doSlimTrkTracks) {
    m_slimTool->slimTrack(*(trk_info.track));
  }
  finalTracks->push_back(std::move(trk_info.track));
  ElementLink<TrackCollection> trackLink(
    *finalTracks, finalTracks->size() - 1, ctx);
  aParticle->setTrackLink(trackLink);
  return StatusCode::SUCCESS;
}
