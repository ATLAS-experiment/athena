/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "egammaUtils/egammaCopyTrackParticleInfo.h"

#include "ActsEMBremCollectionBuilder.h"

using xAOD::EgammaHelpers::summaryValueInt;

ActsEMBremCollectionBuilder::ActsEMBremCollectionBuilder(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode ActsEMBremCollectionBuilder::initialize() {
  ATH_CHECK(m_selectedTrackParticleContainerKey.initialize());

  m_actsTrackLinkKey = m_selectedTrackParticleContainerKey.key() + "." +
                       m_actsTrackLinkKey.key();
  ATH_CHECK(m_actsTrackLinkKey.initialize());
  ATH_CHECK(m_beamSpotKey.initialize());

  ATH_CHECK(m_refittedTracksKey.initialize());
  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_outputTrackParticlesKey.initialize());

  m_actsTrackOutLinkKey = m_outputTrackParticlesKey.key() + "." + m_actsTrackOutLinkKey.key();
  ATH_CHECK(m_actsTrackOutLinkKey.initialize());

  ATH_CHECK(m_actsFitter.retrieve());
  ATH_CHECK(m_cnvTool.retrieve());
  ATH_CHECK(m_trackingGeometryTool.retrieve());

  std::string backendname{};
  try {
    backendname = ActsTrk::prefixFromTrackContainerName(m_refittedTracksKey.key());
  }
  catch (const std::runtime_error &ee){
    backendname = m_refittedTracksKey.key() + "_int";
  }
  ATH_CHECK(m_refittedTracksBackendHandles.initialize(backendname));

  return StatusCode::SUCCESS;
}

StatusCode ActsEMBremCollectionBuilder::finalize() {
  ATH_MSG_INFO("====> GSF fitting Statistics ============");
  ATH_MSG_INFO("Input Tracks: " << m_nInputTracks);
  ATH_MSG_INFO("Output Tracks " << m_nRefittedTracks);
  ATH_MSG_INFO("<========================================");
  return StatusCode::SUCCESS;
}

StatusCode ActsEMBremCollectionBuilder::execute(const EventContext &ctx) const {
  SG::ReadHandle<xAOD::TrackParticleContainer> selectedTrackParticles(
      m_selectedTrackParticleContainerKey, ctx);
  ATH_CHECK(selectedTrackParticles.isValid());
  m_nInputTracks.fetch_add(selectedTrackParticles->size(), std::memory_order_relaxed);

  SG::ReadHandle<xAOD::TrackParticleContainer> originalTPs(
      m_trackParticleContainerKey, ctx);
  ATH_CHECK(originalTPs.isValid());

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle(m_beamSpotKey, ctx);
  ATH_CHECK(beamSpotHandle.isValid());
  const InDet::BeamSpotData* beamSpotData = beamSpotHandle.cptr();

  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::MutableTrackContainer trackContainer( std::move(trackBackend),
                                                 std::move(trackStateBackend) );

  std::vector<const xAOD::TrackParticle *> originals;
  originals.reserve(selectedTrackParticles->size());
  ATH_CHECK(refitActsTracks(ctx, *selectedTrackParticles, trackContainer, originals, beamSpotData));

  // make const
  Acts::ConstVectorTrackContainer ctrackBackend( std::move(trackContainer.container()) );
  Acts::ConstVectorMultiTrajectory ctrackStateBackend( std::move(trackContainer.trackStateContainer()) );
  std::unique_ptr<ActsTrk::TrackContainer> outputActsTracks = std::make_unique<ActsTrk::TrackContainer>( std::move(ctrackBackend),
                                                                                                     std::move(ctrackStateBackend) );

  m_nRefittedTracks.fetch_add(outputActsTracks->size(), std::memory_order_relaxed);

  // Record Acts container first so ElementLinks into it resolve correctly
  SG::WriteHandle<ActsTrk::TrackContainer> refittedTrackHandle = SG::makeHandle(m_refittedTracksKey, ctx);
  if (refittedTrackHandle.record(std::move(outputActsTracks)).isFailure()) {
    ATH_MSG_ERROR("Failed to record refitted ACTS tracks with key "
                  << m_refittedTracksKey.key());
    return StatusCode::FAILURE;
  }
  const ActsTrk::TrackContainer* actsContainer = refittedTrackHandle.ptr();

  // Create output xAOD::TrackParticleContainer
  SG::WriteHandle<xAOD::TrackParticleContainer> outTrackParticleHandle = SG::makeHandle(m_outputTrackParticlesKey, ctx);
  if (outTrackParticleHandle.record(std::make_unique<xAOD::TrackParticleContainer>(),
                      std::make_unique<xAOD::TrackParticleAuxContainer>()).isFailure()) {
    ATH_MSG_ERROR("Failed to record GSF TrackParticles with key "
                  << m_outputTrackParticlesKey.key());
    return StatusCode::FAILURE;
  }
  xAOD::TrackParticleContainer* trackParticles = outTrackParticleHandle.ptr();

  ATH_CHECK(convertTracks(ctx, *actsContainer, originals, *originalTPs, *trackParticles, beamSpotData));

  return StatusCode::SUCCESS;
}

StatusCode ActsEMBremCollectionBuilder::refitActsTracks(
    const EventContext &ctx,
    const xAOD::TrackParticleContainer &input,
    ActsTrk::MutableTrackContainer &trackContainer,
    std::vector<const xAOD::TrackParticle*>& originals,
    const InDet::BeamSpotData* beamSpotData) const {
  // Beam Spot Position
  Acts::Vector3 beamPos( beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
			 beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
			 0 );

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(beamPos);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer,
                      ElementLink<ActsTrk::TrackContainer>>
      decoHandleActsTrackLink(m_actsTrackLinkKey, ctx);

  for (const xAOD::TrackParticle *in : input) {
    int nSiliconHits = summaryValueInt(*in, xAOD::numberOfSCTHits, 0);
    nSiliconHits    += summaryValueInt(*in, xAOD::numberOfPixelHits, 0);
    if (nSiliconHits < m_MinNoSiHits) {
      continue;
    }

    const ElementLink<ActsTrk::TrackContainer> &actsTrackLink =
        decoHandleActsTrackLink(*in);

    if (!actsTrackLink.isValid()) {
      ATH_MSG_WARNING("Invalid ElementLink to ACTS track for track particle ");
      continue;
    }

    std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track =
        *actsTrackLink;
    if (!optional_track.has_value()) {
      ATH_MSG_DEBUG(
          "Could not retrieve track from valid ElementLink for track particle");
      continue;
    }

    ActsTrk::TrackContainer::ConstTrackProxy actstrack = optional_track.value();

    ATH_CHECK(m_actsFitter->fit(ctx, actstrack, trackContainer, *pSurface));
    originals.push_back(in);
  }
  return StatusCode::SUCCESS;
}

StatusCode ActsEMBremCollectionBuilder::convertTracks(
    const EventContext &ctx,
    const ActsTrk::TrackContainer &actsContainer,
    const std::vector<const xAOD::TrackParticle*>& originals,
    const xAOD::TrackParticleContainer& originalTPs,
    xAOD::TrackParticleContainer& outputTPs,
    const InDet::BeamSpotData* beamSpotData) const {
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>>
      actsTrackLink(m_actsTrackOutLinkKey, ctx);

  static const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer>>
      originalTPLink("originalTrackParticle");
      
  static const SG::AuxElement::Accessor<float> QoverPLM("QoverPLM");
  for (const auto [track, originalTP] : Acts::zip(actsContainer, originals)) {
    xAOD::TrackParticle* tp = outputTPs.push_back(std::make_unique<xAOD::TrackParticle>());

    ATH_CHECK(m_cnvTool->convert(*tp, ctx, track, track.referenceSurface(), beamSpotData));

    actsTrackLink(*tp) = ElementLink<ActsTrk::TrackContainer>(&actsContainer, track.index());

    originalTPLink(*tp) = ElementLink<xAOD::TrackParticleContainer>(
        originalTPs, originalTP->index(), ctx);

    // Add qoverP from the last measurement
    float QoverPLast(0);
    for (const auto ts : track.trackStatesReversed()) {
      if (ts.typeFlags().isMeasurement()) {
        QoverPLast = ts.parameters()[Acts::eBoundQOverP];
        break;
      }
    }
    
    QoverPLM(*tp) = QoverPLast;

    // isRefitted option should check the actual refit status
    egammaCopyTrackParticleInfo::ToCopy toCopy{.isRefitted = true,
                                               .doTruth = m_doTruth,
                                               .doPix = m_doPix,
                                               .doSCT = m_doStrip,
                                               .doHGTD = m_doHGTD};
    egammaCopyTrackParticleInfo::copy(*tp, *originalTP, toCopy);

  }
  return StatusCode::SUCCESS;
}
