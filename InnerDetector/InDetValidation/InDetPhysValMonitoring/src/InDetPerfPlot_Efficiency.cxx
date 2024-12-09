/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPerfPlot_Efficiency.h"
#include "xAODTruth/TruthVertex.h"
#include "AthContainers/ConstAccessor.h"
#include "logLinearBinning.h"
#include "GaudiKernel/SystemOfUnits.h" //for Gaudi::Units
using namespace IDPVM;

InDetPerfPlot_Efficiency::InDetPerfPlot_Efficiency(InDetPlotBase* pParent, const std::string& sDir, bool doTechEff) :
  InDetPlotBase(pParent, sDir),
  m_doTechEff(doTechEff) {}

void
InDetPerfPlot_Efficiency::initializePlots() {

  book(m_efficiency_vs_pteta, "efficiency_vs_pteta");
  book(m_efficiency_vs_ptTruthMu, "efficiency_vs_ptTruthMu");
  book(m_efficiency_vs_ptActualMu, "efficiency_vs_ptActualMu");

  book(m_efficiency_vs_etaTruthMu, "efficiency_vs_absEtaTruthMu");
  book(m_efficiency_vs_etaActualMu, "efficiency_vs_absEtaActualMu");

  for(unsigned int i=0; i<m_eta_bins.size()-1; i++){
    m_efficiency_vs_truthMu_eta_bin.emplace_back();
    m_efficiency_vs_actualMu_eta_bin.emplace_back();

    std::string bin_low = std::to_string(m_eta_bins[i]);
    size_t dotPos = bin_low.find('.');
    bin_low.resize(dotPos+2);
    bin_low.replace(dotPos, 1, 1, 'p');
    std::string bin_up = std::to_string(m_eta_bins[i+1]);
    dotPos = bin_up.find('.');
    bin_up.resize(dotPos+2);
    bin_up.replace(dotPos, 1, 1, 'p');

    book(m_efficiency_vs_truthMu_eta_bin.back(),
	 "efficiency_vs_truthMu_absEta_"+bin_low+"_"+bin_up);
    book(m_efficiency_vs_actualMu_eta_bin.back(),
	 "efficiency_vs_actualMu_absEta_"+bin_low+"_"+bin_up);
  }
 
  book(m_efficiency_vs_eta, "efficiency_vs_eta");
  book(m_efficiency_vs_pt, "efficiency_vs_pt");
  book(m_efficiency_vs_pt_low, "efficiency_vs_pt_low");
  book(m_efficiency_vs_pt_high, "efficiency_vs_pt_high");
  book(m_efficiency_vs_lowpt, "efficiency_vs_lowpt");
  book(m_efficiency_vs_phi, "efficiency_vs_phi");
  book(m_efficiency_vs_d0, "efficiency_vs_d0");
  book(m_efficiency_vs_d0_abs, "efficiency_vs_d0_abs");
  book(m_efficiency_vs_z0, "efficiency_vs_z0");
  book(m_efficiency_vs_z0_abs, "efficiency_vs_z0_abs");
  book(m_efficiency_vs_R, "efficiency_vs_R");
  book(m_efficiency_vs_Z, "efficiency_vs_Z");
  book(m_efficiency_vs_truthMu, "efficiency_vs_truthMu");
  book(m_efficiency_vs_actualMu, "efficiency_vs_actualMu");

  if(m_doTechEff){
    book(m_technical_efficiency_vs_eta, "technical_efficiency_vs_eta");
    book(m_technical_efficiency_vs_pt, "technical_efficiency_vs_pt");
    book(m_technical_efficiency_vs_phi, "technical_efficiency_vs_phi");
    book(m_technical_efficiency_vs_d0, "technical_efficiency_vs_d0");
    book(m_technical_efficiency_vs_z0, "technical_efficiency_vs_z0");
    book(m_technical_efficiency_vs_truthMu, "technical_efficiency_vs_truthMu");
    book(m_technical_efficiency_vs_actualMu, "technical_efficiency_vs_actualMu");
  }

  book(m_extended_efficiency_vs_d0, "extended_efficiency_vs_d0");
  book(m_extended_efficiency_vs_d0_abs, "extended_efficiency_vs_d0_abs");
  book(m_extended_efficiency_vs_z0, "extended_efficiency_vs_z0");
  book(m_extended_efficiency_vs_z0_abs, "extended_efficiency_vs_z0_abs");
  book(m_efficiency_vs_prodR, "efficiency_vs_prodR");
  book(m_efficiency_vs_prodR_extended, "efficiency_vs_prodR_extended");
  book(m_efficiency_vs_prodZ, "efficiency_vs_prodZ");
  book(m_efficiency_vs_prodZ_extended, "efficiency_vs_prodZ_extended");

  book(m_TrkRec_eta,       "TrkRec_eta");
  book(m_TrkRec_d0,        "TrkRec_d0");
  book(m_TrkRec_prodR,     "TrkRec_prodR");
  book(m_TrkRec_pT,        "TrkRec_pT");
  book(m_TrkRec_truthMu,   "TrkRec_truthMu");
  book(m_TrkRec_actualMu,  "TrkRec_actualMu");
  book(m_TrkRec_eta_d0,    "TrkRec_eta_d0");
  book(m_TrkRec_eta_prodR, "TrkRec_eta_prodR");
  book(m_TrkRec_eta_pT,    "TrkRec_eta_pT");

  book(m_efficiency_vs_pt_log, "efficiency_vs_pt_log");
  const TH1* h = m_efficiency_vs_pt_log->GetTotalHistogram();
  int nbins = h->GetNbinsX();
  std::vector<double> logptbins = IDPVM::logLinearBinning(nbins, h->GetBinLowEdge(1), h->GetBinLowEdge(nbins + 1), false);
  m_efficiency_vs_pt_log->SetBins(nbins, logptbins.data());
}

void
InDetPerfPlot_Efficiency::fill(const xAOD::TruthParticle& truth, const bool isGood, unsigned int truthMu, float actualMu, float weight) {
  double eta = truth.eta();
  double pt = truth.pt() / Gaudi::Units::GeV; // convert MeV to GeV
  double phi = truth.phi();

  fillHisto(m_efficiency_vs_pteta, pt, eta, isGood, weight);
  fillHisto(m_efficiency_vs_ptTruthMu, pt, truthMu, isGood, weight);
  fillHisto(m_efficiency_vs_ptActualMu, pt, actualMu, isGood, weight);

  fillHisto(m_efficiency_vs_etaTruthMu, std::abs(eta), truthMu, isGood, weight);
  fillHisto(m_efficiency_vs_etaActualMu, std::abs(eta), actualMu, isGood, weight);

  const auto pVal =  std::lower_bound(m_eta_bins.begin(), m_eta_bins.end(), std::abs(eta));
  const int bin = std::distance(m_eta_bins.begin(), pVal) - 1;
  fillHisto(m_efficiency_vs_truthMu_eta_bin[bin], truthMu, isGood, weight);
  fillHisto(m_efficiency_vs_actualMu_eta_bin[bin], actualMu, isGood, weight);

  fillHisto(m_efficiency_vs_eta, eta, isGood, weight);
  fillHisto(m_efficiency_vs_pt, pt, isGood, weight);
  fillHisto(m_efficiency_vs_pt_low, pt, isGood, weight);
  fillHisto(m_efficiency_vs_pt_high, pt, isGood, weight);
  fillHisto(m_efficiency_vs_phi, phi, isGood, weight);
  fillHisto(m_efficiency_vs_pt_log, pt, isGood, weight);
  fillHisto(m_efficiency_vs_lowpt, pt, isGood, weight);

  static const SG::ConstAccessor<float> d0Acc("d0");
  static const SG::ConstAccessor<float> z0Acc("z0");
  static const SG::ConstAccessor<float> prodRAcc("prodR");
  static const SG::ConstAccessor<float> prodZAcc("prodZ");
  double d0 = d0Acc(truth);
  double z0 = z0Acc(truth);
  double R = prodRAcc(truth);
  double Z = prodZAcc(truth);
  fillHisto(m_efficiency_vs_d0, d0, isGood, weight);
  fillHisto(m_efficiency_vs_d0_abs, std::abs(d0), isGood, weight);
  fillHisto(m_efficiency_vs_z0, z0, isGood, weight);
  fillHisto(m_efficiency_vs_z0_abs, std::abs(z0), isGood, weight);
  fillHisto(m_efficiency_vs_R, R, isGood, weight);
  fillHisto(m_efficiency_vs_Z, Z, isGood, weight);

  fillHisto(m_extended_efficiency_vs_d0, d0, isGood, weight);
  fillHisto(m_extended_efficiency_vs_d0_abs, std::abs(d0), isGood, weight);
  fillHisto(m_extended_efficiency_vs_z0, z0, isGood, weight);
  fillHisto(m_extended_efficiency_vs_z0_abs, std::abs(z0), isGood, weight);
  fillHisto(m_efficiency_vs_truthMu, truthMu, isGood, weight);
  fillHisto(m_efficiency_vs_actualMu, actualMu, isGood, weight);

  fillHisto(m_TrkRec_eta, eta, isGood, weight);
  fillHisto(m_TrkRec_d0,  d0,  isGood, weight);
  fillHisto(m_TrkRec_pT,  pt,  isGood, weight);
  fillHisto(m_TrkRec_truthMu, truthMu, isGood, weight);
  fillHisto(m_TrkRec_actualMu, actualMu, isGood, weight);

  fillHisto(m_TrkRec_eta_d0, eta, d0, isGood, weight);
  fillHisto(m_TrkRec_eta_pT, eta, pt, isGood, weight);

  if (truth.hasProdVtx()) {
    const xAOD::TruthVertex* vtx = truth.prodVtx();
    double prod_rad = vtx->perp();
    double prod_z = vtx->z();
    fillHisto(m_efficiency_vs_prodR, prod_rad, isGood, weight);
    fillHisto(m_efficiency_vs_prodR_extended, prod_rad, isGood, weight);
    fillHisto(m_efficiency_vs_prodZ, prod_z, isGood, weight);
    fillHisto(m_efficiency_vs_prodZ_extended, prod_z, isGood, weight);

    fillHisto(m_TrkRec_prodR, prod_rad, isGood, weight);

    fillHisto(m_TrkRec_eta_prodR, eta, prod_rad, isGood, weight);
  }
}

void
InDetPerfPlot_Efficiency::fillTechnicalEfficiency(const xAOD::TruthParticle& truth, const bool isGood, unsigned int truthMu, float actualMu, float weight) {
  double eta = truth.eta();
  double pt = truth.pt() / Gaudi::Units::GeV; // convert MeV to GeV
  double phi = truth.phi();
  fillHisto(m_technical_efficiency_vs_pteta, pt, eta, isGood, weight);
  fillHisto(m_technical_efficiency_vs_eta, eta, isGood, weight);
  fillHisto(m_technical_efficiency_vs_pt, pt, isGood, weight);
  fillHisto(m_technical_efficiency_vs_phi, phi, isGood, weight);

  static const SG::ConstAccessor<float> d0Acc("d0");
  static const SG::ConstAccessor<float> z0Acc("z0");
  double d0 = d0Acc(truth);
  double z0 = z0Acc(truth);
  fillHisto(m_technical_efficiency_vs_d0, d0, isGood, weight);
  fillHisto(m_technical_efficiency_vs_z0, z0, isGood, weight);
  fillHisto(m_technical_efficiency_vs_truthMu, truthMu, isGood, weight);
  fillHisto(m_technical_efficiency_vs_actualMu, actualMu, isGood, weight);
}


void
InDetPerfPlot_Efficiency::finalizePlots() {
}
