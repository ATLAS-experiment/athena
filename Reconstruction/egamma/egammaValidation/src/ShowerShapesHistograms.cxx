/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ShowerShapesHistograms.h"

#include "AsgMessaging/Check.h"
#include "GaudiKernel/ITHistSvc.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "TH1D.h"
#include "TH2D.h"

using namespace egammaMonitoring;

StatusCode ShowerShapesHistograms::initializePlots() {

  const char* fN = m_name.c_str();
  
  histoMap["hadleak"] = new TH1D(Form("%s_hadleak",fN), ";E_{hadleak}; Events"   , 100, -0.07,  0.13  );
  histoMap["rhad"]    = new TH1D(Form("%s_rhad",fN)   , ";R_{had}; Events"       , 100, -0.07,  0.13  );
  histoMap["reta"]    = new TH1D(Form("%s_reta",fN)   , ";R_{#eta}; Events"      , 355,  0.  ,  1.1005);
  histoMap["rphi"]    = new TH1D(Form("%s_rphi",fN)   , ";R_{#phi}; Events"      , 355,  0.  ,  1.1005);
  histoMap["weta2"]   = new TH1D(Form("%s_weta2",fN)  , ";W_{#etas2}; Events"    , 100,  0.  ,  0.03  );
  histoMap["eratio"]  = new TH1D(Form("%s_eratio",fN) , ";E_{ratio}; Events"     , 101,  0.  ,  1.01  );
  histoMap["deltae"]  = new TH1D(Form("%s_deltae",fN) , ";#DeltaE [MeV]; Events" , 100,  0.  ,  500.  );
  histoMap["f1"]      = new TH1D(Form("%s_f1",fN)     , ";f_{1}; Events"         , 100,  0.  ,  1.0   );
  histoMap["fside"]   = new TH1D(Form("%s_fside",fN)  , ";f_{side}; Events"      , 100,  0.  ,  2.0   );
  histoMap["wtots1"]  = new TH1D(Form("%s_wtots1",fN) , ";w_{s, tot}; Events"    , 100,  0.  , 10.    );
  histoMap["ws3"]     = new TH1D(Form("%s_ws3",fN)    , ";w_{s, 3}; Events"      , 100,  0.  ,  1.    );

  histoMap["lateral"]  = new TH1D(Form("%s_lateral",fN),  ";Lateral of seed; Events" , 50,   0.  , 1.     );
  histoMap["second_R"] = new TH1D(Form("%s_second_R",fN), ";Second R; Events"        , 150,  0.  , 15000. );
  histoMap["EMFrac"]   = new TH1D(Form("%s_EMFrac",fN),  ";EMFrac; Events"           , 51,   0.  , 1.02   );
  histo2DMap["lateral_second_R_2D"] = new TH2D(Form("%s_lateral_second_R_2D",fN), ";Lateral of seed; Second R", 50, 0, 1, 150,0.,15000.);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"lateral", histoMap["lateral"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"second_R", histoMap["second_R"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"EMFrac", histoMap["EMFrac"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"lateral_second_R_2D", histo2DMap["lateral_second_R_2D"]));
 
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"rhad", histoMap["rhad"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"hadleak", histoMap["hadleak"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"reta", histoMap["reta"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"rphi", histoMap["rphi"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"weta2", histoMap["weta2"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eratio", histoMap["eratio"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"deltae", histoMap["deltae"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"f1", histoMap["f1"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"fside", histoMap["fside"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"wtots1", histoMap["wtots1"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"ws3", histoMap["ws3"]));

  return StatusCode::SUCCESS;
  
} // initializePlots

void ShowerShapesHistograms::fill(const xAOD::Egamma& egamma) {

  bool isFwd = xAOD::EgammaHelpers::isFwdElectron(&egamma);

  const xAOD::CaloCluster *topoClus(nullptr);
  if (isFwd)
    topoClus = egamma.caloCluster();
  else
    topoClus =
      xAOD::EgammaHelpers::getAssociatedTopoClusters(egamma.caloCluster()).at(0);

  double lateral(0.);
  double emfrac(0.);
  double second_r(0.);
  bool hasBoth = true;
  if (topoClus->retrieveMoment(xAOD::CaloCluster::ENG_FRAC_EM ,emfrac))
    histoMap["EMFrac"]->Fill(emfrac);

  if (topoClus->retrieveMoment(xAOD::CaloCluster::LATERAL,lateral))
    histoMap["lateral"]->Fill(lateral);
  else
    hasBoth = false;

  if (topoClus->retrieveMoment(xAOD::CaloCluster::SECOND_R,second_r))
    histoMap["second_R"]->Fill(second_r);
  else
    hasBoth = false;

  if (hasBoth)
    histo2DMap["lateral_second_R_2D"]->Fill(lateral,second_r);

  // maybe L2, L3 shapes (e.g. Reta, Rphi, weta2) could be computed for electron in innerwheel,
  // but certainly not L1
  if (!isFwd) {

    float eta2 = std::abs(egamma.caloCluster()->etaBE(2));
    float rhad = -999;
    float Reta = -999;
    float Rphi = -999;
    float shweta2 = -999;
    float Eratio = -999;
    float DeltaE = -999;
    float frac_f1 = -999;
    float shfside = -999;
    float shwtots1 = -999;
    float shws3 = -999;

    if (egamma.showerShapeValue(rhad, xAOD::EgammaParameters::Rhad)) {
      histoMap["rhad"]->Fill(rhad);
      float rhad1 = -999;
      if (egamma.showerShapeValue(rhad1, xAOD::EgammaParameters::Rhad1)) {
	float hadrleak = (eta2 >= 0.8 && eta2 < 1.37) ? rhad : rhad1;
	histoMap["hadleak"]->Fill(hadrleak);
      }
    }
    if (egamma.showerShapeValue(Reta, xAOD::EgammaParameters::Reta))
      histoMap["reta"]->Fill(Reta);
    if (egamma.showerShapeValue(Rphi, xAOD::EgammaParameters::Rphi))
      histoMap["rphi"]->Fill(Rphi);
    if (egamma.showerShapeValue(shweta2, xAOD::EgammaParameters::weta2))
      histoMap["weta2"]->Fill(shweta2);
    if (egamma.showerShapeValue(frac_f1, xAOD::EgammaParameters::f1))
      histoMap["f1"]->Fill(frac_f1);
    // Only fill S1 variables if there some energy in S1 (to avoid under/overflows
    if (frac_f1 != 0) {
      if (egamma.showerShapeValue(Eratio, xAOD::EgammaParameters::Eratio))
	histoMap["eratio"]->Fill(Eratio);
      if (egamma.showerShapeValue(DeltaE, xAOD::EgammaParameters::DeltaE))
	histoMap["deltae"]->Fill(DeltaE);
      if (egamma.showerShapeValue(shfside, xAOD::EgammaParameters::fracs1))
	histoMap["fside"]->Fill(shfside);
      if (egamma.showerShapeValue(shwtots1, xAOD::EgammaParameters::wtots1))
	histoMap["wtots1"]->Fill(shwtots1);
      if (egamma.showerShapeValue(shws3, xAOD::EgammaParameters::weta1))
	histoMap["ws3"]->Fill(shws3);
    }
  }
}
