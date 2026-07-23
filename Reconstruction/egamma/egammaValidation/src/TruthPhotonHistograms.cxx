/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthPhotonHistograms.h"

#include "AsgTools/AnaToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "xAODBase/IParticle.h"
#include "xAODTruth/TruthParticle.h" 
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaTruthxAODHelpers.h"

#include "TH1D.h"
#include "TH2D.h"

using namespace egammaMonitoring;

StatusCode TruthPhotonHistograms::initializePlots() {

  ATH_CHECK(ParticleHistograms::initializePlots());

  const char* fN = m_name.c_str();

  histoMap["convRadius_all"]       = new TH1D(Form("%s_convRadius_all",fN),       ";Conversion Radius [mm]; Conversion Radius Events", 200, 0, 2000);
  histoMap["convRadius"]           = new TH1D(Form("%s_convRadius",fN),           ";Conversion Radius [mm]; Conversion Radius Events", 14, m_cR_bins);
  histoMap["convRadius_15GeV"]     = new TH1D(Form("%s_convRadius_15GeV",fN),     ";Conversion Radius [mm]; Conversion Radius Events", 14, m_cR_bins);
  histoMap["convRadiusTrueVsReco"] = new TH1D(Form("%s_convRadiusTrueVsReco",fN), ";R^{reco}_{conv. vtx} - R^{true}_{conv. vtx} [mm]; Events", 100, -200, 200);

  histoMap["pileup"]       = new TH1D(Form("%s_pileup",fN),       ";mu; mu Events", 35, 0., 70.);
  histoMap["pileup_15GeV"] = new TH1D(Form("%s_pileup_15GeV",fN), ";mu; mu Events", 35, 0., 70.);
  histoMap["onebin"]       = new TH1D(Form("%s_onebin",fN),       "; ; Events", 1, 0., 1.);
  histoMap["onebin_15GeV"] = new TH1D(Form("%s_onebin_15GeV",fN), "; ; Events", 1, 0., 1.);

  histoMap["resolution_e"]   = new TH1D(Form("%s_resolution_e",fN),   "; E_{reco} / E_{true} - 1; Events", 40, -0.2, 0.2);
  histoMap["resolution_eta"] = new TH1D(Form("%s_resolution_eta",fN), "; #eta_{reco} - #eta_{true}; Events", 20, -0.05, 0.05);
  histoMap["resolution_phi"] = new TH1D(Form("%s_resolution_phi",fN), "; #phi_{reco} - #phi_{true}; Events", 20, -0.05, 0.05);

  histo2DMap["resolution_e_vs_pT"]  = new TH2D(Form("%s_resolution_e_vs_pT",fN),  ";p_{T} [GeV];E_{reco} / E_{true} - 1", 40, 0, 200, 160, -0.2, 0.2);
  histo2DMap["resolution_e_vs_eta"] = new TH2D(Form("%s_resolution_e_vs_eta",fN), ";|#eta|;E_{reco} / E_{true} - 1", 25, 0, 2.5, 160, -0.2, 0.2);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"convRadius_all", histoMap["convRadius_all"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"convRadius", histoMap["convRadius"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"convRadius_15GeV", histoMap["convRadius_15GeV"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"convRadiusTrueVsReco", histoMap["convRadiusTrueVsReco"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"pileup", histoMap["pileup"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"pileup_15GeV", histoMap["pileup_15GeV"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"onebin", histoMap["onebin"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"onebin_15GeV", histoMap["onebin_15GeV"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"resolution_e", histoMap["resolution_e"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"resolution_eta", histoMap["resolution_eta"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"resolution_phi", histoMap["resolution_phi"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"resolution_e_vs_pT", histo2DMap["resolution_e_vs_pT"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"resolution_e_vs_eta", histo2DMap["resolution_e_vs_eta"]));

  return StatusCode::SUCCESS;

}

void TruthPhotonHistograms::fill(const xAOD::IParticle& phrec) {
  TruthPhotonHistograms::fill(phrec,0.);
}

void TruthPhotonHistograms::fill(const xAOD::IParticle& phrec, float mu) {

  ParticleHistograms::fill(phrec);

  const xAOD::TruthParticle *truth  = xAOD::TruthHelpers::getTruthParticle(phrec);

  float trueR = -999;
  if (truth) {
    if (truth->pdgId() == 22 && truth->hasDecayVtx()) {
      trueR = truth->decayVtx()->perp();
    }
  }

  histoMap["convRadius_all"]->Fill(trueR);
  histoMap["convRadius"]->Fill(trueR);
  histoMap["pileup"]->Fill(mu);
  histoMap["onebin"]->Fill(0.5);

  if(phrec.pt()/1000. > 15) {
    histoMap["convRadius_15GeV"]->Fill(trueR);
    histoMap["pileup_15GeV"]->Fill(mu);
    histoMap["onebin_15GeV"]->Fill(0.5);
  }

  // access reco photon from the xAOD::TruthParticle (can't use the IParticle* here)
  const auto *truthParticle = dynamic_cast<const xAOD::TruthParticle*>(&phrec);
  if (truth && truthParticle) {
    const xAOD::Photon *photon = xAOD::EgammaHelpers::getRecoPhoton(truthParticle);

    if (photon) {
      float res_e = photon->e()/truth->e() - 1.;
      float res_eta = photon->eta() - truth->eta();
      float res_phi = photon->phi() - truth->phi();
      float recoR = xAOD::EgammaHelpers::conversionRadius(photon);

      // Resolution histograms only make sense if there was a reco particle !
      // And for resolution on R, only if true conv, reco conv
      if (recoR < 1000 && trueR > 0)
	histoMap["convRadiusTrueVsReco"]->Fill(recoR - trueR);

      histoMap["resolution_e"]->Fill(res_e);
      histoMap["resolution_eta"]->Fill(res_eta);
      histoMap["resolution_phi"]->Fill(res_phi);

      histo2DMap["resolution_e_vs_pT"]->Fill(phrec.pt()/1000., res_e);
      histo2DMap["resolution_e_vs_eta"]->Fill(std::abs(phrec.eta()), res_e);

    }
  }


} // fill
