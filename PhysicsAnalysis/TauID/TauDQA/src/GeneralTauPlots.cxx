/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeneralTauPlots.h"
#include "AthContainers/ConstAccessor.h"
#include "AthenaKernel/Units.h"

namespace Tau{

GeneralTauPlots::GeneralTauPlots(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName):
   PlotBase(pParent, sDir),
   m_oTauKinematicPlots(this, "", sTauJetContainerName),
   m_sTauJetContainerName(sTauJetContainerName)
{	
}

GeneralTauPlots::~GeneralTauPlots()
{
}

void GeneralTauPlots::initializePlots(){

   m_tauCharge      = Book1D("Charge",m_sTauJetContainerName + " Tau charge; charge; # Taus",7,-3.,4.);
   m_tauNChargedTracks = Book1D("NChargedTracks", m_sTauJetContainerName + " Tau n charged tracks; nCharged; # Taus",10,0.,10.);
   m_tauNIsolatedTracks = Book1D("NIsolatedTracks", m_sTauJetContainerName + " Tau n isolated tracks; nIsolated; # Taus",10,0.,10.);
   m_tauNCoreTracks = Book1D("NCoreTracks",m_sTauJetContainerName + " Tau n core tracks; nCore; # Taus",15,0.,15.);
   m_tauNWideTracks = Book1D("NWideTracks",m_sTauJetContainerName + " Tau n wide tracks; nWide; # Taus",20,0.,20.);
   m_ptHighPt = Book1D("ptHighPt", m_sTauJetContainerName+" HighPt"+"; pt; # Taus",20, 0.0, 1500.0);
   m_RNNEleScore = Book1D("RNNEleScore", m_sTauJetContainerName+" RNNEleScore;RNNEleScore;# Tau", 50,0.,1.);
   m_RNNEleScoreSigTrans = Book1D("RNNEleScoreSigTrans", m_sTauJetContainerName+" RNNEleScoreSigTrans;RNNEleScoreSigTrans;"+"# Tau", 50,0.,1.);
   m_RNNJetScore = Book1D("RNNJetScore", m_sTauJetContainerName+" RNNJetScore;RNNJetScore;# Tau", 50,0.,1.);
   m_RNNJetScoreSigTrans = Book1D("RNNJetScoreSigTrans", m_sTauJetContainerName+" RNNJetScoreSigTrans;RNNJetScoreSigTrans;"+"# Tau", 50,0.,1.);
   m_GNTauScore = Book1D("GNTauScore", m_sTauJetContainerName+" GNTauScore;GNTauScore;# Tau", 50,0.,1.);
   m_GNTauScoreSigTrans = Book1D("GNTauScoreSigTrans", m_sTauJetContainerName+" GNTauScoreSigTrans;GNTauScoreSigTrans;"+"# Tau", 50,0.,1.);
   m_ptRNNLoose = Book1D("ptRNNSigLoose",m_sTauJetContainerName+" RNNSigLoose; pt; # Taus", 20, 0.0, 150.0);
   m_ptRNNLooseHighPt = Book1D("ptRNNSigLooseHighPt", m_sTauJetContainerName+" RNNSigLooseHighPt; pt"+"; # Taus",20, 0.0, 1500.0);
   m_ptRNNMedium = Book1D("ptRNNSigMedium",m_sTauJetContainerName+" RNNSigMedium; pt; # Taus", 20, 0.0, 150.0);
   m_ptRNNMediumHighPt = Book1D("ptRNNSigMediumHighPt", m_sTauJetContainerName+" RNNSigMediumHighPt; pt"+"; # Taus",20, 0.0, 1500.0);
   m_ptRNNTight = Book1D("ptRNNSigTight",m_sTauJetContainerName+" RNNSigTight; pt; # Taus", 20, 0.0, 150.0);
   m_ptRNNTightHighPt = Book1D("ptRNNSigTightHighPt", m_sTauJetContainerName+" RNNSigTightHighPt; pt"+"; # Taus",20, 0.0, 1500.0);
   m_ptGNTauLoose = Book1D("ptGNTauSigLoose",m_sTauJetContainerName+" GNTauSigLoose; pt; # Taus", 20, 0.0, 150.0);
   m_ptGNTauLooseHighPt = Book1D("ptGNTauSigLooseHighPt", m_sTauJetContainerName+" GNTauSigLooseHighPt; pt"+"; # Taus",20, 0.0, 1500.0);
   m_ptGNTauMedium = Book1D("ptGNTauSigMedium",m_sTauJetContainerName+" GNTauSigMedium; pt; # Taus", 20, 0.0, 150.0);
   m_ptGNTauMediumHighPt = Book1D("ptGNTauSigMediumHighPt", m_sTauJetContainerName+" GNTauSigMediumHighPt; pt"+"; # Taus",20, 0.0, 1500.0);
   m_ptGNTauTight = Book1D("ptGNTauSigTight",m_sTauJetContainerName+" GNTauSigTight; pt; # Taus", 20, 0.0, 150.0);
   m_ptGNTauTightHighPt = Book1D("ptGNTauSigTightHighPt", m_sTauJetContainerName+" GNTauSigTightHighPt; pt"+"; # Taus",20, 0.0, 1500.0);



}
  
void GeneralTauPlots::fill(const xAOD::TauJet& tau, float weight) {
  m_oTauKinematicPlots.fill(tau, weight);
  m_tauCharge->Fill(tau.charge(), weight); 
  m_tauNChargedTracks->Fill(tau.nTracks(), weight);
  m_tauNIsolatedTracks->Fill(tau.nTracks(xAOD::TauJetParameters::classifiedIsolation), weight);
  m_tauNCoreTracks->Fill(tau.nTracks(xAOD::TauJetParameters::coreTrack), weight);
  m_tauNWideTracks->Fill(tau.nTracks(xAOD::TauJetParameters::wideTrack), weight); 
  m_ptHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);

  static const SG::ConstAccessor<float> acc_RNNEleScore("RNNEleScore");
  if ( acc_RNNEleScore.isAvailable(tau) ) {
     float rnnScore = tau.discriminant(xAOD::TauJetParameters::RNNEleScore);
     if ( rnnScore > -2.0 ) m_RNNEleScore->Fill(rnnScore, weight);
  }
  static const SG::ConstAccessor<float> acc_RNNEleScoreSigTrans("RNNEleScoreSigTrans_v1");
  if ( acc_RNNEleScoreSigTrans.isAvailable(tau) ) {
     float rnnScore = acc_RNNEleScoreSigTrans(tau);
     m_RNNEleScoreSigTrans->Fill(rnnScore, weight);
  }
  static const SG::ConstAccessor<float> acc_RNNJetScore("RNNJetScore");
  if ( acc_RNNJetScore.isAvailable(tau) ) {
     float rnnScore = tau.discriminant(xAOD::TauJetParameters::RNNJetScore);
     if ( rnnScore > -2.0 ) m_RNNJetScore->Fill(rnnScore, weight);
  }
  static const SG::ConstAccessor<float> acc_RNNJetScoreSigTrans("RNNJetScoreSigTrans");
  if ( acc_RNNJetScoreSigTrans.isAvailable(tau) ) {
     float rnnScore = tau.discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
     m_RNNJetScoreSigTrans->Fill(rnnScore, weight);
  }
  static const SG::ConstAccessor<float> acc_GNTauScore("GNTauScore_v0prune");
  if ( acc_GNTauScore.isAvailable(tau) ) {
     float gntauScore = acc_GNTauScore(tau);
     if ( gntauScore > -2.0 ) m_GNTauScore->Fill(gntauScore, weight);
  }
  static const SG::ConstAccessor<float> acc_GNTauScoreSigTrans("GNTauScoreSigTrans_v0prune");
  if ( acc_GNTauScoreSigTrans.isAvailable(tau) ) {
     float gntauScoreSigTrans = acc_GNTauScoreSigTrans(tau);
     if ( gntauScoreSigTrans > -2.0 ) m_GNTauScoreSigTrans->Fill(gntauScoreSigTrans, weight);
  }

  if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigLoose) ) {
     m_ptRNNLoose      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptRNNLooseHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }
  if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigMedium) ) {
     m_ptRNNMedium      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptRNNMediumHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }
  if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigTight) ) {
     m_ptRNNTight      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptRNNTightHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }

  static const SG::ConstAccessor<char> acc_GNTauL("GNTauL_v0prune");
  if( acc_GNTauL.isAvailable(tau) && acc_GNTauL(tau)) {
     m_ptGNTauLoose      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptGNTauLooseHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }
  static const SG::ConstAccessor<char> acc_GNTauM("GNTauM_v0prune");
  if( acc_GNTauM.isAvailable(tau) && acc_GNTauM(tau)) {
     m_ptGNTauMedium      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptGNTauMediumHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }
  static const SG::ConstAccessor<char> acc_GNTauT("GNTauT_v0prune");
  if( acc_GNTauT.isAvailable(tau) && acc_GNTauT(tau)) {
     m_ptGNTauTight      ->Fill(tau.pt()/Athena::Units::GeV, weight);
     m_ptGNTauTightHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
  }
}


}
