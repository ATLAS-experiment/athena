/*
  Get muon ID tracks from L2CB muons
  
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GetL2CBmuonInDetTracksAlg.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

GetL2CBmuonInDetTracksAlg::GetL2CBmuonInDetTracksAlg(const std::string& name, ISvcLocator* pSvcLocator )
:AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode GetL2CBmuonInDetTracksAlg::initialize(){

  ATH_CHECK(m_muonL2CBContainerKey.initialize());
  ATH_CHECK(m_idTrackOutputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode GetL2CBmuonInDetTracksAlg::execute(const EventContext& ctx) const
{
  
  SG::WriteHandle<xAOD::TrackParticleContainer> wh_outidtracks(m_idTrackOutputKey, ctx);
  ATH_CHECK(wh_outidtracks.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));
  xAOD::TrackParticleContainer *idtracksout = wh_outidtracks.ptr();

  SG::ReadHandle<xAOD::L2CombinedMuonContainer> cbMuons(m_muonL2CBContainerKey, ctx);
  ATH_CHECK(cbMuons.isPresent());
  ATH_MSG_DEBUG("adding combined muon container with size: "<<cbMuons->size());

  for(auto cbmuon : *cbMuons) {
    if(cbmuon->idTrack()) idtracksout->push_back(new xAOD::TrackParticle(*cbmuon->idTrack()));
  }
  ATH_MSG_DEBUG("output ID muon tracks with size: " << idtracksout->size());
  return StatusCode::SUCCESS;

}
