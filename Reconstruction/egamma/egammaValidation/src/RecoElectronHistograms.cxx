/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoElectronHistograms.h"
#include "AsgMessaging/Check.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH2D.h"
#include "TH3D.h"

using namespace egammaMonitoring;

StatusCode RecoElectronHistograms::initializePlots() {

  ATH_CHECK(ParticleHistograms::initializePlots());

  const char* fN = m_name.c_str();

  if (!m_isData) {
    histoMap["truthType"]   = new TH1D(Form("%s_truthType",fN),   ";truth type; Events",  41,-1,40);
    histoMap["truthOrigin"] = new TH1D(Form("%s_truthOrigin",fN), ";truth origin; Events",51,-1,50);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"truthType",   histoMap["truthType"]));
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"truthOrigin", histoMap["truthOrigin"]));
  }

  histo2DMap["eta_nTracks"] = new TH2D(Form("%s_eta_nTracks",fN),
      ";#eta;n_{trk}; Events", 90, -4.5, 4.5, 10, 0,10);
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eta_nTracks", histo2DMap["eta_nTracks"]));

  histo3DMap["eteta_eop"] = new TH3D(Form("%s_eteta_eop",fN),
      ";E_{T} [GeV];#eta;E/p; Events", 20, 0, 200, 25, 0, 2.5, 250, 0.5,3.);
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eteta_eop", histo3DMap["eteta_eop"]));

  return StatusCode::SUCCESS;

} 

void RecoElectronHistograms::fill(const xAOD::Electron& elrec) {

  ParticleHistograms::fill(elrec);

  if (!m_isData) {
    static const SG::AuxElement::ConstAccessor<int> accType("truthType");
    static const SG::AuxElement::ConstAccessor<int> accOrigin("truthOrigin");
    if (accOrigin.isAvailable(elrec))
      histoMap["truthOrigin"]->Fill(accOrigin(elrec));
    else
      histoMap["truthOrigin"]->Fill(-1);
    if (accType.isAvailable(elrec))
      histoMap["truthType"]->Fill(accType(elrec));
    else
      histoMap["truthType"]->Fill(-1);
  }

  histo2DMap["eta_nTracks"]->Fill(elrec.eta(),elrec.nTrackParticles());
  if (elrec.nTrackParticles())
    histo3DMap["eteta_eop"]->Fill(elrec.pt()*1e-3,std::abs(elrec.eta()),elrec.pt()/elrec.trackParticle()->pt());

}
  
