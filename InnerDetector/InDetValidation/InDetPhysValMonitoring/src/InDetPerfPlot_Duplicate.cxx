/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPerfPlot_Duplicate.h"
#include "GaudiKernel/SystemOfUnits.h" //for Gaudi::Units

InDetPerfPlot_Duplicate::InDetPerfPlot_Duplicate
(InDetPlotBase* pParent, const std::string& sDir) :
  InDetPlotBase(pParent, sDir){
  // nop
}

void InDetPerfPlot_Duplicate::initializePlots() {

  book(m_rate_vs_pt, "duplicate_rate_vs_pt");
  book(m_rate_vs_eta, "duplicate_rate_vs_eta");

  book(m_number_vs_pt, "duplicate_number_vs_pt");
  book(m_number_vs_eta, "duplicate_number_vs_eta");

  book(m_number_nonzero_vs_pt, "duplicate_number_nonzero_vs_pt");
  book(m_number_nonzero_vs_eta, "duplicate_number_nonzero_vs_eta");

}

void InDetPerfPlot_Duplicate::fill
(const xAOD::TruthParticle& truth, unsigned int ntracks, float weight) {
  double eta = truth.eta();
  double pt = truth.pt() / Gaudi::Units::GeV; // convert MeV to GeV

  fillHisto(m_rate_vs_pt, pt, ntracks>1, weight);
  fillHisto(m_rate_vs_eta, eta, ntracks>1, weight);

  m_number_vs_pt->Fill(pt, ntracks, weight);
  m_number_vs_eta->Fill(eta, ntracks, weight);

  if(ntracks>0){
    m_number_nonzero_vs_pt->Fill(pt, ntracks, weight);
    m_number_nonzero_vs_eta->Fill(eta, ntracks, weight);
  }
}

  
