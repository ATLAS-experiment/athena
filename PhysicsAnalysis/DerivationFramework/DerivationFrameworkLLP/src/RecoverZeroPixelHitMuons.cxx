/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkLLP/RecoverZeroPixelHitMuons.h"

RecoverZeroPixelHitMuons::RecoverZeroPixelHitMuons(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode RecoverZeroPixelHitMuons::initialize()
{
  ATH_CHECK(m_inputMuonContainerKey.initialize());
  ATH_CHECK(m_inputTrackContainerKey.initialize());
  ATH_CHECK(m_outputMuonContainerKey.initialize());
    
  return StatusCode::SUCCESS;
}


StatusCode RecoverZeroPixelHitMuons::execute(const EventContext& context) const
{

  SG::ReadHandle<xAOD::MuonContainer> inputMuons{m_inputMuonContainerKey, context};
  if( ! inputMuons.isValid() ) {
    ATH_MSG_ERROR ("Couldn't retrieve xAOD::MuonContainer with key: " << m_inputMuonContainerKey.key() );
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::TrackParticleContainer> inputTracks{m_inputTrackContainerKey, context};
  if( ! inputTracks.isValid() ) {
    ATH_MSG_ERROR ("Couldn't retrieve xAOD::TrackParticleContainer with key: " << m_inputTrackContainerKey.key() );
    return StatusCode::FAILURE;
  }

  auto outputMuons = std::make_unique<xAOD::MuonContainer>();
  auto outputMuonsAux = std::make_unique<xAOD::MuonAuxContainer>();

  outputMuons->setStore (outputMuonsAux.get());

  xAOD::TrackParticleContainer::const_iterator trItr;
  double track_eta=0.0;
  double track_phi=0.0;
  double track_charge=0.0;
  double mu_eta=0.0;
  double mu_phi=0.0;
  double mu_charge=0.0;
  std::vector<const xAOD::Muon*> matchedMuons;
  int n_tracks=0;
  for (const xAOD::TrackParticle *t : *inputTracks){
    n_tracks++;
    // if we have a pixel hit, should reconstruct these guys as combined muons
    uint8_t nPixHits = 0;
    t->summaryValue(nPixHits,xAOD::numberOfPixelHits);
    if (nPixHits > 0) continue;
    track_eta=t->eta();
    track_phi=t->phi();
    track_charge=t->charge();
    // loop over muons 
    xAOD::MuonContainer::const_iterator muItr;
    float min_dR=999.;
    float min_mu_charge=0.0;
    xAOD::MuonContainer::const_iterator muMatchItr;

    for (muItr = inputMuons->begin(); muItr != inputMuons->end(); ++muItr){
      const xAOD::Muon &m = *(*muItr);
      if (m.muonType() != xAOD::Muon::MuonStandAlone) continue;
      if (std::find(matchedMuons.begin(), matchedMuons.end(), *muItr) != matchedMuons.end()) continue;
          
      mu_eta=m.eta();
      mu_phi=m.phi();
      mu_charge=m.charge();
      float mu_dR=std::sqrt(std::pow(mu_eta-track_eta,2)+std::pow(mu_phi-track_phi,2));  
      if (mu_dR < min_dR){
        min_dR=mu_dR;
        min_mu_charge=mu_charge;
        muMatchItr=muItr;
      }
    }
    if ((min_dR < m_matchingDeltaR) && (min_mu_charge == track_charge)){
      matchedMuons.push_back(*muMatchItr);
      const xAOD::Muon &muon_match = *(*muMatchItr);      
      xAOD::Muon *zeroPixelHitMuon = new xAOD::Muon(muon_match);
      outputMuons->push_back(zeroPixelHitMuon);      
      zeroPixelHitMuon->setP4(muon_match.pt(),track_eta,track_phi);
      zeroPixelHitMuon->setCharge(min_mu_charge);
      // Set to not assigned Muon Type to distinguish from other muons, will need to properly define in MuonType enum later
      zeroPixelHitMuon->setMuonType((xAOD::Muon::MuonType)10);
      ElementLink<xAOD::TrackParticleContainer> link( *inputTracks, n_tracks-1 );
      zeroPixelHitMuon->setTrackParticleLink(xAOD::Muon::InnerDetectorTrackParticle, link);
    }
    
  }

  SG::WriteHandle<xAOD::MuonContainer> outputMuonsHandle{m_outputMuonContainerKey, context};

  CHECK( outputMuonsHandle.record (std::move(outputMuons), std::move (outputMuonsAux)) );


  
  return StatusCode::SUCCESS;
}

