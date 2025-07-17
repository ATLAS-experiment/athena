/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>
#include <algorithm>

#include "CorePlots.h"
#include "AthContainers/ConstAccessor.h"

#include "AthenaKernel/Units.h"

namespace DiTau{

  CorePlots::CorePlots(PlotBase* pParent, const std::string& sDir, std::string sDiTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sDiTauJetContainerName(std::move(sDiTauJetContainerName))
  {
  }
	
  CorePlots::~CorePlots()
  {
  }

  void CorePlots::initializePlots(){

    pt       = Book1D("pt" , "DiTau pt; DiTau Transverse Momentum [GeV];Entries / 1 GeV",25,0.,200);
    eta      = Book1D("eta", "DiTau eta; DiTau Pseudo-Rapidity;Entries / 0.05", 32, -3.2, 3.2);
    phi      = Book1D("phi", "DiTau phi; DiTau Azimuthal Angle;Entries / 0.05", 32, -3.2, 3.2);
    mass     = Book1D("mass", "DiTau mass; DiTau Mass; Entries / 10", 100,0,200);
    nsubjets = Book1D("nsubjets", "DiTau #subjets; DiTau #subjets; Entries", 10,0,10);
    charge   = Book1D("charge", "DiTau charge; DiTau charge; Entries", 10,-5,5);

    eta_pt  = Book2D("eta_pt","DiTau eta vs pt;DiTau eta; DiTau pt; Entries.0.05/1 GeV",32,-3.2,3.2,25,0.,200);
    eta_phi = Book2D("eta_phi","DiTau eta vs phi;DiTau eta;DiTau phi;Entries.0.05/0.5",32,-3.2,3.2,32,-3.2,3.2);

    lead_subjet_pt  = Book1D("lead_subjet_pt", "DiTau leading subjet pt; DiTau Leading Subjet Transverse Momentum [GeV];Entries / 1 GeV",20,0.,100);
    lead_subjet_eta = Book1D("lead_subjet_eta", "DiTau leading subjet eta; DiTau Leading Subjet Pseudo-Rapidity;Entries / 0.05", 32, -3.2, 3.2);
    lead_subjet_phi = Book1D("lead_subjet_phi", "DiTau leading subjet phi; DiTau Leading Subjet Azimuthal Angle;Entries / 0.05", 32, -3.2, 3.2);

    sublead_subjet_pt  = Book1D("sublead_subjet_pt", "DiTau subleading subjet pt; DiTau Subleading Subjet Transverse Momentum [GeV];Entries / 1 GeV",10,0.,50);
    sublead_subjet_eta = Book1D("sublead_subjet_eta", "DiTau subleading subjet eta; DiTau Subleading Subjet Pseudo-Rapidity;Entries / 0.05", 32, -3.2, 3.2);
    sublead_subjet_phi = Book1D("sublead_subjet_phi", "DiTau subleading subjet phi; DiTau Subleading Subjet Azimuthal Angle;Entries / 0.05", 32, -3.2, 3.2);
		    
  }

  void CorePlots::fill(const xAOD::DiTauJet& ditau, float weight) {

     pt->Fill(ditau.pt()/Athena::Units::GeV, weight);
     eta->Fill(ditau.eta(), weight);
     phi->Fill(ditau.phi(), weight);
     mass->Fill(ditau.m()/Athena::Units::GeV, weight);
     nsubjets->Fill(ditau.nSubjets(), weight);

     eta_pt->Fill(ditau.eta(), ditau.pt()/Athena::Units::GeV, weight);
     eta_phi->Fill(ditau.eta(), ditau.phi(), weight);

     int ditau_charge = 0;
     for (const auto& xTrack : ditau.trackLinks()) {
        if (!xTrack.isValid())
           continue;

	if(ditau.nSubjets() >= 2){
           for (int i = 0; i < 2; ++i) { // loop over two leading subjets 
              TLorentzVector tlvSubjet = TLorentzVector();
              tlvSubjet.SetPtEtaPhiE(ditau.subjetPt(i), ditau.subjetEta(i),
                                     ditau.subjetPhi(i), ditau.subjetE(i));
              double dR = tlvSubjet.DeltaR((*xTrack)->p4());
              if (dR < 0.1) {
                 ditau_charge += (*xTrack)->charge();
                 break; //prevents double counting of tracks
              }
           }  // loop over subjets
        }	   
     } // loop over tracks

     charge->Fill(ditau_charge,weight);

     // subjet histograms
     lead_subjet_pt->Fill(ditau.subjetPt(0)/Athena::Units::GeV,weight);
     lead_subjet_eta->Fill(ditau.subjetEta(0),weight);
     lead_subjet_phi->Fill(ditau.subjetPhi(0),weight);

     sublead_subjet_pt->Fill(ditau.subjetPt(1)/Athena::Units::GeV,weight);
     sublead_subjet_eta->Fill(ditau.subjetEta(1),weight);
     sublead_subjet_phi->Fill(ditau.subjetPhi(1),weight);

  }
}
