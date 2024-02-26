/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "EMBremCollectionBuilder.h"

#include "AthenaKernel/errorcheck.h"
#include "TrkTrack/LinkToTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrackLink/ITrackLink.h"
#include "TrkTrackSummary/TrackSummary.h"

#include "TrkMaterialOnTrack/EstimatedBremOnTrack.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// std includes
#include <algorithm>
#include <memory>

using xAOD::EgammaHelpers::summaryValueInt;

namespace {
  void copySummaryValue(
    const xAOD::TrackParticle &src, 
    xAOD::TrackParticle &dest, 
    const xAOD::SummaryType &information
  ) {
    uint8_t value = summaryValueInt(src, information, 0);
    dest.setSummaryValue(value, information);
  }
}

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

  ATH_MSG_INFO("Not refitted due to Silicon Requirements "
               << m_FailedSiliconRequirFit);
  ATH_MSG_INFO("Failed Fit Tracks " << m_FailedFitTracks);
  ATH_MSG_INFO("RefittedTracks " << m_RefittedTracks);

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
    int nSiliconHits_trk = summaryValueInt(*trackParticle, xAOD::numberOfSCTHits, 0);
    nSiliconHits_trk += summaryValueInt(*trackParticle, xAOD::numberOfPixelHits, 0);
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

  // Do the GSF refit. Note that the output is  two collections
  // (for fit success or failure) of TrackWithIndex.
  // TrackWithIndex means a Trk::::Track
  // and the index to the original TrackParticle Collection .
  // Note that altough the input is a xAOD::TrackParticle
  // what we really refit is the corresponding Trk::Track.
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

  copyOverInfo(*aParticle, *original, isRefitted);
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

void
EMBremCollectionBuilder::copyOverInfo(xAOD::TrackParticle& created,
                                      const xAOD::TrackParticle& original,
                                      bool isRefitted) const
{
  // Add Truth decorations. Copy from the original.
  if (m_doTruth) {
    static const SG::AuxElement::Accessor<
      ElementLink<xAOD::TruthParticleContainer>>
      tPL("truthParticleLink");
    if (tPL.isAvailable(original)) {
      tPL(created) = tPL(original);
    }
    static const SG::AuxElement::Accessor<float> tMP("truthMatchProbability");
    if (tMP.isAvailable(original)) {
      tMP(created) = tMP(original);
    }
    static const SG::AuxElement::Accessor<int> tT("truthType");
    if (tT.isAvailable(original)) {
      tT(created) = tT(original);
    }
    static const SG::AuxElement::Accessor<int> tO("truthOrigin");
    if (tO.isAvailable(original)) {
      tO(created) = tO(original);
    }
  }
  
  copySummaryValue(original, created, xAOD::numberOfPixelSplitHits);
  copySummaryValue(original, created, xAOD::numberOfInnermostPixelLayerSplitHits);
  copySummaryValue(original, created, xAOD::numberOfNextToInnermostPixelLayerSplitHits);
  copySummaryValue(original, created, xAOD::numberOfPixelSharedHits);
  copySummaryValue(original, created, xAOD::numberOfInnermostPixelLayerSharedHits);
  copySummaryValue(original, created, xAOD::numberOfNextToInnermostPixelLayerSharedHits);
  copySummaryValue(original, created, xAOD::numberOfSCTSharedHits);
  copySummaryValue(original, created, xAOD::numberOfTRTSharedHits);

  if (m_doHGTD) {
    created.setHasValidTime(original.hasValidTime());
    created.setTime(original.time());
  }

  if (isRefitted) {
    if (m_doPix) {
      // copy over dead sensors
      copySummaryValue(original, created, xAOD::numberOfPixelDeadSensors);

      // Figure the new number of holes
      uint8_t nPixHolesRefitted = 
        - summaryValueInt(created, xAOD::numberOfPixelHits, -1)
        - summaryValueInt(created, xAOD::numberOfPixelOutliers, -1)
        + summaryValueInt(original, xAOD::numberOfPixelHits, -1)
        + summaryValueInt(original, xAOD::numberOfPixelOutliers, -1)
        + summaryValueInt(original, xAOD::numberOfPixelHoles, -1);

      created.setSummaryValue(nPixHolesRefitted, xAOD::numberOfPixelHoles);
    }
    if (m_doSCT) {
      // Copy over dead and double holes
      copySummaryValue(original, created, xAOD::numberOfSCTDeadSensors);
      copySummaryValue(original, created, xAOD::numberOfSCTDoubleHoles);

      uint8_t nSCTHolesRefitted = 
        - summaryValueInt(created, xAOD::numberOfSCTHits, -1)
        - summaryValueInt(created, xAOD::numberOfSCTOutliers, -1)
        + summaryValueInt(original, xAOD::numberOfSCTHits, -1)
        + summaryValueInt(original, xAOD::numberOfSCTHoles, -1)
        + summaryValueInt(original, xAOD::numberOfSCTOutliers, -1);

      created.setSummaryValue(nSCTHolesRefitted, xAOD::numberOfSCTHoles);
    }
    if (m_doTRT) {
      uint8_t nTRTHolesRefitted = 
        - summaryValueInt(created, xAOD::numberOfTRTHits, -1)
        - summaryValueInt(created, xAOD::numberOfTRTOutliers, -1)
        + summaryValueInt(original, xAOD::numberOfTRTHits, -1)
        + summaryValueInt(original, xAOD::numberOfTRTHoles, -1)
        + summaryValueInt(original, xAOD::numberOfTRTOutliers, -1);

      created.setSummaryValue(nTRTHolesRefitted, xAOD::numberOfTRTHoles);
    }
  }
}

