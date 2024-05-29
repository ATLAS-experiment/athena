/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackHistograms.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AsgMessaging/Check.h"
#include "xAODBase/IParticle.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "TH1D.h"
#include "TProfile.h"

using namespace egammaMonitoring;
using xAOD::EgammaHelpers::summaryValueFloat;
using xAOD::EgammaHelpers::summaryValueInt;

StatusCode TrackHistograms::initializePlots() {

  const char* fN = m_name.c_str();
  
  histoMap["pT"] = new TH1D(Form("%s_pT",fN), ";p_{T} [GeV];Tracks", 100, 0., 100.);

  histoMap["PrecisionHitFraction"]        = new TH1D(Form("%s_PrecisionHitFraction",fN),        ";Precision hit fraction;Tracks", 20, 0., 1.);
  histoMap["PrecisionHitFraction_lowmu"]  = new TH1D(Form("%s_PrecisionHitFraction_lowmu",fN),  ";Precision hit fraction;Tracks", 20, 0., 1.);
  histoMap["PrecisionHitFraction_highmu"] = new TH1D(Form("%s_PrecisionHitFraction_highmu",fN), ";Precision hit fraction;Tracks", 20, 0., 1.);

  histoMap["eProbabilityHT"]        = new TH1D(Form("%s_eProbabilityHT",fN),        ";eProbabilityHT;Tracks", 20, 0., 1.);
  histoMap["eProbabilityHT_lowmu"]  = new TH1D(Form("%s_eProbabilityHT_lowmu",fN),  ";eProbabilityHT;Tracks", 20, 0., 1.);
  histoMap["eProbabilityHT_highmu"] = new TH1D(Form("%s_eProbabilityHT_highmu",fN), ";eProbabilityHT;Tracks", 20, 0., 1.);

  profileMap["PrecisionHitFractionvsmu"] = new TProfile(Form("%s_PrecisionHitFractionvsmu",fN), ";mu;Precision hit fraction", 35, 0., 70., 0., 1.);
  profileMap["eProbabilityHTvsmu"]       = new TProfile(Form("%s_eProbabilityHTvsmu",fN),       ";mu;eProbabilityHT", 35, 0., 70., 0., 1.);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"pT", histoMap["pT"]));
 
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"PrecisionHitFraction",        histoMap["PrecisionHitFraction"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"PrecisionHitFraction_lowmu",  histoMap["PrecisionHitFraction_lowmu"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"PrecisionHitFraction_highmu", histoMap["PrecisionHitFraction_highmu"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eProbabilityHT",        histoMap["eProbabilityHT"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eProbabilityHT_lowmu",  histoMap["eProbabilityHT_lowmu"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eProbabilityHT_highmu", histoMap["eProbabilityHT_highmu"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"PrecisionHitFractionvsmu", profileMap["PrecisionHitFractionvsmu"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eProbabilityHTvsmu",       profileMap["eProbabilityHTvsmu"]));

  return StatusCode::SUCCESS;
  
} // initializePlots


void TrackHistograms::fill(const xAOD::IParticle& track) {
  TrackHistograms::fill(track,0.);
}

void
TrackHistograms::fill(const xAOD::IParticle& track, float mu)
{

  const xAOD::TrackParticle* tp =
    dynamic_cast<const xAOD::TrackParticle*>(&track);

  int nTRTHits = summaryValueInt(*tp, xAOD::numberOfTRTHits);
  int nTRTTubeHits = summaryValueInt(*tp, xAOD::numberOfTRTTubeHits);
  float precHitFrac = (nTRTHits > 0 && nTRTTubeHits >= 0)
                        ? (1. - ((float)nTRTTubeHits) / ((float)nTRTHits))
                        : -999.;

  float eProbabilityHT = summaryValueFloat(*tp, xAOD::eProbabilityHT);

  histoMap["pT"]->Fill(tp->pt() / 1000.);

  histoMap["PrecisionHitFraction"]->Fill(precHitFrac);
  histoMap["eProbabilityHT"]->Fill(eProbabilityHT);

  if (mu < 25.) {
    histoMap["PrecisionHitFraction_lowmu"]->Fill(precHitFrac);
    histoMap["eProbabilityHT_lowmu"]->Fill(eProbabilityHT);
  }

  if (mu > 35.) {
    histoMap["PrecisionHitFraction_highmu"]->Fill(precHitFrac);
    histoMap["eProbabilityHT_highmu"]->Fill(eProbabilityHT);
  }

  profileMap["PrecisionHitFractionvsmu"]->Fill(mu, precHitFrac);
  profileMap["eProbabilityHTvsmu"]->Fill(mu, eProbabilityHT);
}
