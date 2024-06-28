/*
  Merge muon ID track containers from inside-out and outside-in reconstruction
  
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MergeMuonInDetTracksAlg.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

MergeMuonInDetTracksAlg::MergeMuonInDetTracksAlg(const std::string& name, ISvcLocator* pSvcLocator )
:AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode MergeMuonInDetTracksAlg::initialize(){

  ATH_CHECK(m_fullIDtrackContainerKey.initialize());
  renounce(m_muonCBContainerKey);
  ATH_CHECK(m_muonCBContainerKey.initialize());
  renounce(m_muonInsideOutContainerKey);
  ATH_CHECK(m_muonInsideOutContainerKey.initialize());
  renounce(m_muonL2mtContainerKey);
  ATH_CHECK(m_muonL2mtContainerKey.initialize());
  ATH_CHECK(m_idTrackOutputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode MergeMuonInDetTracksAlg::execute(const EventContext& ctx) const
{
  
  SG::WriteHandle<xAOD::TrackParticleContainer> wh_outidtracks(m_idTrackOutputKey, ctx);
  ATH_CHECK(wh_outidtracks.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));
  xAOD::TrackParticleContainer *idtracksout = wh_outidtracks.ptr();

  SG::ReadHandle<xAOD::L2CombinedMuonContainer> cbMuons(m_muonCBContainerKey, ctx);
  ATH_CHECK(cbMuons.isPresent());
  ATH_MSG_DEBUG("adding combined muon container with size: "<<cbMuons->size());

  SG::ReadHandle<xAOD::L2CombinedMuonContainer> ioMuons(m_muonInsideOutContainerKey, ctx);
  ATH_CHECK(ioMuons.isPresent());
  ATH_MSG_DEBUG("adding inside-out muon container with size: "<<ioMuons->size());

  SG::ReadHandle<xAOD::L2CombinedMuonContainer> l2mtMuons(m_muonL2mtContainerKey, ctx);
  ATH_CHECK(l2mtMuons.isPresent());
  ATH_MSG_DEBUG("adding L2 multi-track muon container with size: "<<l2mtMuons->size());

  SG::ReadHandle<xAOD::TrackParticleContainer> FullIDtracks(m_fullIDtrackContainerKey, ctx);
  ATH_CHECK(FullIDtracks.isPresent());
  ATH_MSG_DEBUG("full container of ID tracks with size: "<<FullIDtracks->size());
  for(auto track : *FullIDtracks) {
    bool trackMatcher = false;
    // check the cb muon id tracks
    for(auto cbmuon : *cbMuons) {
      if(!cbmuon->idTrack()) continue;
      if(cbmuon->idTrack()==track) {
	trackMatcher = true;
	break;
      }
    }

    // check the l2io muon id tracks
    for(auto iomuon : *ioMuons) {
      if(!iomuon->idTrack()) continue;
      if(iomuon->idTrack()==track) {
	trackMatcher = true;
	break;
      }
    }

    // check the l2mt muon id tracks
    for(auto l2mtmuon : *l2mtMuons) {
      if(!l2mtmuon->idTrack()) continue;
      if(l2mtmuon->idTrack()==track) {
	trackMatcher = true;
	break;
      }
    } 

    // add the track to the ID track container if it matches an ID track from the full collection
    if(trackMatcher) {
      idtracksout->push_back(new xAOD::TrackParticle(*track));
    }
    else {
      ATH_MSG_DEBUG("no inner detector tracks could be matched to any cb, io or l2mt muon in the event");
    }
  }
  ATH_MSG_DEBUG("output ID muon tracks with size: " << idtracksout->size());
  return StatusCode::SUCCESS;

}
