/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"

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
  ATH_CHECK(m_actsFitter.retrieve());
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

  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::MutableTrackContainer trackContainer( std::move(trackBackend),
                                                 std::move(trackStateBackend) );

  std::vector<const xAOD::TrackParticle *> siliconTrackParticles;
  siliconTrackParticles.reserve(16);

  for (const xAOD::TrackParticle *trackParticle : *selectedTrackParticles) {
    int nSiliconHits_trk =
        summaryValueInt(*trackParticle, xAOD::numberOfSCTHits, 0);
    nSiliconHits_trk +=
        summaryValueInt(*trackParticle, xAOD::numberOfPixelHits, 0);
    if (nSiliconHits_trk >= m_MinNoSiHits) {
      siliconTrackParticles.push_back(trackParticle);
    }
  }

  ATH_CHECK(refitActsTracks(ctx, siliconTrackParticles, trackContainer));

  // make const
  Acts::ConstVectorTrackContainer ctrackBackend( std::move(trackContainer.container()) );
  Acts::ConstVectorMultiTrajectory ctrackStateBackend( std::move(trackContainer.trackStateContainer()) );
  std::unique_ptr<ActsTrk::TrackContainer> outputTracks = std::make_unique<ActsTrk::TrackContainer>( std::move(ctrackBackend),
                                                                                                     std::move(ctrackStateBackend) );

  SG::WriteHandle<ActsTrk::TrackContainer> refittedTrackHandle = SG::makeHandle(m_refittedTracksKey, ctx);

  if (refittedTrackHandle.record(std::move(outputTracks)).isFailure()) {
    ATH_MSG_ERROR("Failed to record refitted ACTS tracks with key "
                  << m_refittedTracksKey.key());
    return StatusCode::FAILURE;
  }

  const size_t inputCount = selectedTrackParticles->size();
  const size_t refittedCount = trackContainer.size();

  m_nInputTracks.fetch_add(inputCount, std::memory_order_relaxed);
  m_nRefittedTracks.fetch_add(refittedCount, std::memory_order_relaxed);

  return StatusCode::SUCCESS;
}

StatusCode ActsEMBremCollectionBuilder::refitActsTracks(
    const EventContext &ctx,
    const std::vector<const xAOD::TrackParticle *> &input,
    ActsTrk::MutableTrackContainer &trackContainer) const {
  // Get Beam pos and make pSurface
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle( m_beamSpotKey, ctx );
  ATH_CHECK( beamSpotHandle.isValid() );
  const InDet::BeamSpotData* beamSpotData = beamSpotHandle.cptr();
  
  // Beam Spot Position
  Acts::Vector3 beamPos( beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
			 beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
			 0 );

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(beamPos);

  
  for (const xAOD::TrackParticle *in : input) {
    SG::ReadDecorHandle<xAOD::TrackParticleContainer,
                        ElementLink<ActsTrk::TrackContainer>>
        decoHandleActsTrackLink(m_actsTrackLinkKey, ctx);
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

    ATH_CHECK(m_actsFitter->fit(ctx, actstrack, trackContainer, *pSurface.get()));
  }
  return StatusCode::SUCCESS;
}
