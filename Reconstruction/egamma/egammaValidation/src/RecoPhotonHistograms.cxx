/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoPhotonHistograms.h"

#include "AsgMessaging/Check.h"
#include "GaudiKernel/ITHistSvc.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include "TH1D.h"

using namespace egammaMonitoring;

StatusCode RecoPhotonHistograms::initializePlots() {

  ATH_CHECK(ParticleHistograms::initializePlots());

  const char* fN = m_name.c_str();

  histoMap["convRadius"]  = new TH1D(Form("%s_convRadius",fN),  ";Conversion Radius [mm]; Events", 14, m_cR_bins);
  histoMap["truthType"]   = new TH1D(Form("%s_truthType",fN),   ";truth type; Events",  41,-1,40);
  histoMap["truthOrigin"] = new TH1D(Form("%s_truthOrigin",fN), ";truth origin; Events",51,-1,50);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"convRadius",  histoMap["convRadius"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"truthType",   histoMap["truthType"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"truthOrigin", histoMap["truthOrigin"]));

  return StatusCode::SUCCESS;
}

void RecoPhotonHistograms::fill(const xAOD::Photon& phrec) {
  
  ParticleHistograms::fill(phrec);

  static const SG::AuxElement::ConstAccessor<int> accType("truthType");
  static const SG::AuxElement::ConstAccessor<int> accOrigin("truthOrigin");
  if (accOrigin.isAvailable(phrec))
    histoMap["truthOrigin"]->Fill(accOrigin(phrec));
  else
    histoMap["truthOrigin"]->Fill(-1);
  if (accType.isAvailable(phrec))
    histoMap["truthType"]->Fill(accType(phrec));
  else
    histoMap["truthType"]->Fill(-1);
 
  double trueR(-999);
  const xAOD::TruthParticle *tmp = xAOD::TruthHelpers::getTruthParticle(phrec);
  if (tmp) {
    if (tmp->pdgId() == 22 && tmp->hasDecayVtx()) {

      float x = tmp->decayVtx()->x();
      float y = tmp->decayVtx()->y();
      trueR = std::sqrt( x*x + y*y );

    }
  }
  histoMap["convRadius"]->Fill(trueR);

  
} // fill
