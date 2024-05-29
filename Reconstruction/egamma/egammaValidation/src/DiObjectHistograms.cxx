/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "DiObjectHistograms.h"
#include "xAODBase/IParticle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AsgMessaging/Check.h"
#include "TH1D.h"
#include "TH2D.h"

namespace egammaMonitoring {

  StatusCode DiObjectHistograms::initializePlots() {

    const char* fN = m_name.c_str();

    histoMap["mass"] = new TH1D(Form("%s_mass",fN),";m [GeV]; Events",300,50,200);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"mass", histoMap["mass"]));

    histoMap["massvsmu"] = new TH2D(Form("%s_massvsmu",fN),";#mu;m [GeV]",100,0,100,150,50,200);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"massvsmu", histoMap["massvsmu"]));

    histoMap["massvspT"] = new TH2D(Form("%s_massvspT",fN),";p_{T} [GeV];m [GeV]",20,0,100,150,50,200);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+"massvspT", histoMap["massvspT"]));
    return StatusCode::SUCCESS;
  }

  void DiObjectHistograms::fill(const xAOD::IParticle& eg1, const xAOD::IParticle& eg2) {
    fill(eg1,eg2,0.);
  }

  void DiObjectHistograms::fill(const xAOD::IParticle& eg1, const xAOD::IParticle& eg2, float mu) {

    xAOD::IParticle::FourMom_t di = eg1.p4()+eg2.p4();
    double m = di.M()*1e-3;
    histoMap["mass"]->Fill(m);
    histoMap["massvsmu"]->Fill(mu,m);
    histoMap["massvspT"]->Fill(di.Pt()*1e-3,m);

  }

}
