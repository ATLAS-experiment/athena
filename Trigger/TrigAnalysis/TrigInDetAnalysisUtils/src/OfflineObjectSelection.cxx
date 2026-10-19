/**
 **     @file    OfflineObjectSelection.cxx
 **
 **     @author  mark sutton
 **     @date    Thu 10 Jan 2019 20:12:10 CET 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TrkTrackSummary/TrackSummary.h"
#include "TrigInDetAnalysisUtils/OfflineObjectSelection.h"


// xAOD offline electron selection
// selection: 0 take all; 1,4 tight; 2,5 medium; 3,6 loose; <4 cut based, >3 likelihood
bool TIDA::isGoodOffline(const xAOD::Electron& elec, const unsigned int selection, double ETOffline ) {

  double sintheta = std::sin( 2*std::atan(std::exp(-elec.eta())) );
  double et = elec.e()*sintheta;
  if ( std::fabs(et)<ETOffline ) return false;

  //  if ( std::fabs(elec.e())<ETOffline ) return false; 
  if      (selection == 1) return elec.passSelection("Tight");
  else if (selection == 2) return elec.passSelection("Medium");
  else if (selection == 3) return elec.passSelection("Loose");
  else if (selection == 4) return elec.passSelection("LHTight");  
  else if (selection == 5) return elec.passSelection("LHMedium");
  else if (selection == 6) return elec.passSelection("LHLoose");
  return true; 
}

// Currently no xAOD offline muon selection implemented
bool TIDA::isGoodOffline(const xAOD::Muon& muon, const unsigned int selection, double ) { 

  /// should allow this to be set also
  if ( muon.muonType() != xAOD::Muon::MuonType::Combined ) return false;
 
  bool good_muon = false;

  /// This muon selection is *ridiculous* - it looks like if it passed "Tight" 
  /// selection, then it *does not* pass medium selection, as the goups are *exclusive*
  /// this is completely nonsensical !

  if      ( selection == 1 ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Tight);
  else if ( selection == 2 ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Medium);
  else if ( selection == 3 ) good_muon = (muon.quality()<=xAOD::Muon::Quality::Loose);
  else if ( selection == 4 ) good_muon = (muon.quality()<=xAOD::Muon::Quality::VeryLoose);
  else                       good_muon = true;

  return good_muon; 
}


// xAOD offline tau selection
// selection: 0 = take all, 1 = tight, 2 = medium, 3 = loose, any other unsigned int = no JetBDTSig requirement
bool TIDA::isGoodOffline(const xAOD::TauJet& tau, const unsigned int selection, int requireNtracks, double EtCutOffline ) {

  //  still require the ET threshold and the track multiplicity
  //  if ( selection==0) return true;

  /// hmmm	           
  TLorentzVector TauTLV = tau.p4();
  double        eta_Tau = TauTLV.Eta();
  double         et_Tau = TauTLV.Et();

  if ( std::fabs(eta_Tau)>2.47 )        return false;
  if ( std::fabs(et_Tau)<EtCutOffline ) return false;

  //  int ntrack_Tau = tau.nTracks();

#ifdef XAODTRACKING_TRACKPARTICLE_H
#ifndef XAODTAU_VERSIONS_TAUJET_V3_H
  int Ntracks = tau.nTracks();
  //  std::cout << "SUTT no tau detail " << Ntracks << "\t(isGood)" << std::endl;
#else
  int Ntracks=0;
  tau.detail( xAOD::TauJetParameters::nChargedTracks, Ntracks );
  //  std::cout << "SUTT tau detail: N " << Ntracks << "\t(isGood)" << std::endl;
#endif
#else
  int Ntracks = tau.numTrack();
#endif

  // Could this selection just be replaced by a string that is passed in? Or an enumerate if the function is needed to be called when incrementing an int for the selection?
  bool good_tau = false;

#if 0
  /// run 2 selection
  if      ( selection == 1 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetBDTSigTight);
  else if ( selection == 2 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetBDTSigMedium);
  else if ( selection == 3 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetBDTSigLoose);
  else                       good_tau = true;
#else
  /// run 3 selection
  if      ( selection == 1 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigTight);
  else if ( selection == 2 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigMedium);
  else if ( selection == 3 ) good_tau = tau.isTau(xAOD::TauJetParameters::JetRNNSigLoose);
  else                       good_tau = true;
#endif


  /// instead of a bool to decide on whether 1 or 3 tracks are required, 
  /// why not just specify the number of tracks ?
  //  if ( doThreeProng==false && Ntracks!=1 ) return false;
  //  if ( doThreeProng==true  && Ntracks!=3 ) return false;
  if ( requireNtracks>0 && Ntracks!=requireNtracks ) return false;

  //bool not_a_electron = !( tauisTau(xAOD::TauJetParameters::EleBDTMedium) );
  //bool not_a_muon = !( tauisTau(xAOD::TauJetParameters::MuonVeto) );
  //bool best_tau = good_tau && not_a_electron && not_a_muon;

  return good_tau;
}
