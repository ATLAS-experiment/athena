/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkLLP/RecoverZeroPixelHitMuons.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <TVector3.h>

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

  std::vector<const xAOD::Muon*> matchedMuons;
  for (const xAOD::TrackParticle *t : *inputTracks){
    // if we have a pixel hit, should reconstruct these guys as combined muons
    uint8_t nPixHits = 0;
    t->summaryValue(nPixHits,xAOD::numberOfPixelHits);
    if (nPixHits > 0) continue;

    // loop over muons 
    float min_dR=999.;
    const xAOD::Muon* muMatch = nullptr;

    for (const xAOD::Muon* m : *inputMuons){
      if (m->muonType() != xAOD::Muon::MuonType::MuonStandAlone) continue;
      if (std::find(matchedMuons.begin(), matchedMuons.end(), m) != matchedMuons.end()) continue;

      // Define SA muon vector
      float mu_dR= xAOD::P4Helpers::deltaR(t->eta(), t->phi(), m->eta(), m->phi());

      // Update matching between ID track and SA muon
      if (mu_dR < min_dR){
        min_dR=mu_dR;
        muMatch=m;
      }
    }
    if ((min_dR < m_matchingDeltaR) && (muMatch->charge() == t->charge())){
      matchedMuons.push_back(muMatch);
      const xAOD::Muon &muon_match = *muMatch;
      xAOD::Muon *zeroPixelHitMuon = outputMuons->push_back(std::make_unique<xAOD::Muon>(muon_match));
      outputMuons->push_back(zeroPixelHitMuon);      
      zeroPixelHitMuon->setP4(muon_match.pt(), t->eta(), t->phi());
      zeroPixelHitMuon->setCharge(muon_match.charge());
      zeroPixelHitMuon->setMuonType(xAOD::Muon::MuonType::ZeroPixelHit);
      ElementLink<xAOD::TrackParticleContainer> link( *inputTracks, t->index() );
      zeroPixelHitMuon->setTrackParticleLink(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle, link);
    }
    
  }

  SG::WriteHandle<xAOD::MuonContainer> outputMuonsHandle{m_outputMuonContainerKey, context};

  CHECK( outputMuonsHandle.record (std::move(outputMuons), std::move (outputMuonsAux)) );


  
  return StatusCode::SUCCESS;
}

