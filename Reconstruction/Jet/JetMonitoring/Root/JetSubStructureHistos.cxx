/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/JetSubStructureHistos.h"
#include "TString.h"
#include <map>
#include "TH1F.h"

#define toGeV 1/1000.

JetSubStructureHistos::JetSubStructureHistos(const std::string &t) : JetHistoBase(t) 
                                                             ,m_jetScale("JetAssignedScaleMomentum")
{
    declareProperty("JetScale", m_jetScale);
}

int JetSubStructureHistos::buildHistos(){

  // -------------- Modify histo names/titles --------
  //  if we're plotting a defined scale, modify the Histo titles and names
  
  std::map<std::string, std::string> scale2str( { 
      { "JetEMScaleMomentum" , "EMScale" } , 
      { "JetConstitScaleMomentum" , "ConstitScale" } } );
  TString scaleTag = scale2str[ m_jetScale ] ; // defaults to ""

  TString prefixn = scaleTag;
  if(prefixn != "") prefixn +="_";
  // -------------- 

  // Build and register the histos in this group : 
  TH1::AddDirectory(kFALSE); // Turn off automatic addition to gDirectory to avoid Warnings. Histos are anyway placed in their own dir later.
  m_tau21     = bookHisto( new TH1F(prefixn+"Tau21"  ,  "Jet Tau21 ;Entries", 100, 0, 1) );
  m_tau32     = bookHisto( new TH1F(prefixn+"Tau32"  ,  "Jet Tau32 ;Entries", 100, 0, 1) );
  m_tau21_wta = bookHisto( new TH1F(prefixn+"Tau21_wta"  ,  "Jet Tau21_wta ;Entries", 100, 0, 1) );
  m_tau32_wta = bookHisto( new TH1F(prefixn+"Tau32_wta"  ,  "Jet Tau32_wta ;Entries", 100, 0, 1) );
  m_C1        = bookHisto( new TH1F(prefixn+"C1"  ,  "Jet C1;Entries", 100, -1, 1) );
  m_C2        = bookHisto( new TH1F(prefixn+"C2"  ,  "Jet C2;Entries", 100, -1, 1) );
  m_D2        = bookHisto( new TH1F(prefixn+"D2"  ,  "Jet D2;Entries", 100, 0, 10) );
  


  TH1::AddDirectory(kTRUE); // Turn on automatic addition to gDirectory in case others needs it.

  // -------------- Modify histo titles.
  if(prefixn != "" ){

    // build a qualifier in the form "(EMScale, Leading Jet, ...)"
    TString qualif = "(";
    TString tags[] = { scaleTag};
    for(const auto& t : tags ) { if(qualif != "(") qualif+=",";qualif += t; }
    qualif += ")";
    // reset all titles :
    for(auto& hdata : m_vBookedHistograms ){
      TString t = hdata.hist->GetTitle(); t+=" "+qualif;
      hdata.hist->SetTitle(t );
    }
  }

  
  return 0;
}



int JetSubStructureHistos::fillHistosFromJet(const xAOD::Jet &j, float weight){
  //For definitions see JetSubStructureMomentTools

  static const SG::ConstAccessor<float> Tau1Acc("Tau1");
  static const SG::ConstAccessor<float> Tau2Acc("Tau2");
  static const SG::ConstAccessor<float> Tau3Acc("Tau3");
  static const SG::ConstAccessor<float> Tau1_wtaAcc("Tau1_wta");
  static const SG::ConstAccessor<float> Tau2_wtaAcc("Tau2_wta");
  static const SG::ConstAccessor<float> Tau3_wtaAcc("Tau3_wta");
  static const SG::ConstAccessor<float> ECF1Acc("ECF1");
  static const SG::ConstAccessor<float> ECF2Acc("ECF2");
  static const SG::ConstAccessor<float> ECF3Acc("ECF3");
  if( Tau1Acc.isAvailable(j) && Tau2Acc.isAvailable(j) && Tau3Acc.isAvailable(j)){
    if( Tau1Acc(j) > 1e-8 ) m_tau21->Fill( Tau2Acc(j) / Tau1Acc(j), weight );
    if( Tau2Acc(j) > 1e-8 ) m_tau32->Fill( Tau3Acc(j) / Tau2Acc(j), weight );
  }
  if( Tau1_wtaAcc.isAvailable(j) && Tau2_wtaAcc(j) && Tau3_wtaAcc(j)){
    if( Tau1_wtaAcc(j) > 1e-8 ) m_tau21_wta->Fill( Tau2_wtaAcc(j) / Tau1_wtaAcc(j), weight );
    if( Tau2_wtaAcc(j) > 1e-8 ) m_tau32_wta->Fill( Tau3_wtaAcc(j) / Tau2_wtaAcc(j), weight );
  }
  if( ECF1Acc.isAvailable(j) && ECF2Acc.isAvailable(j) && ECF3Acc.isAvailable(j)){
    if( ECF1Acc(j) > 1e-8 ) m_C1->Fill( ECF2Acc(j) / pow( ECF1Acc(j), 2.0), weight );
    if( ECF2Acc(j) > 1e-8 ) {
      m_C2->Fill( ( ECF3Acc(j) * ECF1Acc(j) ) / pow( ECF2Acc(j), 2.0), weight );
      m_D2->Fill( ( ECF3Acc(j) * pow( ECF1Acc(j), 3.0 ) ) / pow( ECF2Acc(j), 3.0), weight );
    }
  }

  return 0;
}

