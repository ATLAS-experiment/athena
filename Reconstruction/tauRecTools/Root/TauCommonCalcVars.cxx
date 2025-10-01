/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
// file:        TauCommonCalcVars.cxx
// package:     Reconstruction/tauRec
// authors:     Stan Lai
// date:        2008-05-18
// 
// This class calculates tau variables after core seed reconstruction           
//-----------------------------------------------------------------------------
#include "tauRecTools/TauCommonCalcVars.h"
#include <vector>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

TauCommonCalcVars::TauCommonCalcVars(const std::string &name) :
  TauRecToolBase(name) {
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

TauCommonCalcVars::~TauCommonCalcVars() {
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------
StatusCode TauCommonCalcVars::execute(xAOD::TauJet& pTau) const {

  /////////////////////////////////////////////////
  // Calculate variables that are always valid   
  ////////////////////////////////////////////////

  //init some vars
  pTau.setDetail( xAOD::TauJetParameters::SumPtTrkFrac, 0.f );

  // Leading track pT and et/pt(lead track)
  if (pTau.nTracks() > 0) {
    pTau.setDetail( xAOD::TauJetParameters::leadTrkPt, static_cast<float>( pTau.track(0)->pt() ) );
      
    float emscale_ptEM = 0.;
    float emscale_ptHad = 0.;
      
    if ( !pTau.detail( xAOD::TauJetParameters::etEMAtEMScale, emscale_ptEM ) ) 
      {
	ATH_MSG_DEBUG("retrieval of tau detail failed. stopping calculation of further variables");
	return StatusCode::SUCCESS;
      }

    if ( !pTau.detail( xAOD::TauJetParameters::etHadAtEMScale, emscale_ptHad ) )
      {
	ATH_MSG_DEBUG("retrieval of tau detail failed. stopping calculation of further variables");
	return StatusCode::SUCCESS;
      }
      
    pTau.setDetail( xAOD::TauJetParameters::etOverPtLeadTrk, static_cast<float>( (emscale_ptEM + emscale_ptHad) / pTau.track(0)->pt() ) );
  }

  std::vector<const xAOD::TauTrack*> tauTracks = pTau.tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged);
  for( const xAOD::TauTrack* trk : pTau.tracks((xAOD::TauJetParameters::TauTrackFlag) m_isolationTrackType.value()) ) tauTracks.push_back(trk);
  if (!tauTracks.empty()) {

    TLorentzVector sumOfTrackVector;
    double ptSum = 0;
    double ptSum_altcalc = 0;
    double innerPtSum = 0;
    double sumWeightedDR_tautrack = 0;
    double sumWeightedDR_leadtracktrack = 0;
    double innerSumWeightedDR = 0;
    double sumWeightedDR2_tautrack = 0;
    double sumWeightedDR2_leadtracktrack = 0;

    for (const xAOD::TauTrack* tauTrk : tauTracks){
      sumOfTrackVector += tauTrk->p4();

      double deltaR_tautrack = inTrigger() ? pTau.p4().DeltaR(tauTrk->p4()) : pTau.p4(xAOD::TauJetParameters::IntermediateAxis).DeltaR(tauTrk->p4());

      ptSum += tauTrk->pt();
      sumWeightedDR_tautrack += deltaR_tautrack * tauTrk->pt();
      sumWeightedDR2_tautrack += deltaR_tautrack * deltaR_tautrack * tauTrk->pt();

      //add calculation of innerTrkAvgDist
      if(tauTrk->flag(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged)){
        innerPtSum += tauTrk->pt();
        innerSumWeightedDR += deltaR_tautrack * tauTrk->pt();
      }

      if (tauTracks.size()> 1 && pTau.nTracks()>0) {
        ptSum_altcalc += tauTrk->pt();	      
        double deltaR_leadtracktrack = pTau.track(0)->p4().DeltaR(tauTrk->p4());
        sumWeightedDR_leadtracktrack += deltaR_leadtracktrack * tauTrk->pt();
        sumWeightedDR2_leadtracktrack += deltaR_leadtracktrack * deltaR_leadtracktrack * tauTrk->pt();
      }

    }
    // invariant mass of track system
    pTau.setDetail( xAOD::TauJetParameters::massTrkSys, static_cast<float>( sumOfTrackVector.M() ) );

    if (ptSum > 0.) {
      // seedCalo_trkAvgDist
      pTau.setDetail( xAOD::TauJetParameters::trkAvgDist, static_cast<float>( sumWeightedDR_tautrack / ptSum ) );

      // seedCalo_trkRmsDist
      double trkRmsDist2 = sumWeightedDR2_tautrack / ptSum - pow(sumWeightedDR_tautrack/ptSum, 2.);
      if (trkRmsDist2 > 0.) {
        pTau.setDetail( xAOD::TauJetParameters::trkRmsDist, static_cast<float>( std::sqrt(trkRmsDist2) ) );
      }
      else {
        pTau.setDetail( xAOD::TauJetParameters::trkRmsDist, 0.f );
      }

      // SumPtTrkFrac
      pTau.setDetail( xAOD::TauJetParameters::SumPtTrkFrac, static_cast<float>( 1. - innerPtSum/ptSum ) );
    }
    else {
      pTau.setDetail( xAOD::TauJetParameters::trkAvgDist, 0.f );
      pTau.setDetail( xAOD::TauJetParameters::SumPtTrkFrac, 0.f );
    }

    if (innerPtSum > 0.) {
      // InnerTrkAvgDist
      pTau.setDetail( xAOD::TauJetParameters::innerTrkAvgDist, static_cast<float>( innerSumWeightedDR / innerPtSum ) );
    }
    else {
      pTau.setDetail( xAOD::TauJetParameters::innerTrkAvgDist, 0.f );
    }

    if (tauTracks.size()> 1 && pTau.nTracks()>0) {
      double trkWidth2 = (ptSum_altcalc!=0.) ? (sumWeightedDR2_leadtracktrack/ptSum_altcalc - std::pow(sumWeightedDR_leadtracktrack/ptSum_altcalc, 2.)) : 0.;

      if (trkWidth2 > 0.) pTau.setDetail( xAOD::TauJetParameters::trkWidth2, static_cast<float>( trkWidth2 ) );
      else pTau.setDetail( xAOD::TauJetParameters::trkWidth2, 0.f );
    }
  }

  return StatusCode::SUCCESS;
}
