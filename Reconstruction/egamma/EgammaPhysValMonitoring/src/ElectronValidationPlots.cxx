/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronValidationPlots.h"
#include "xAODEgamma/EgammaDefs.h"

ElectronValidationPlots::ElectronValidationPlots(PlotBase* pParent, const std::string& sDir):PlotBase(pParent, sDir),
										      m_oCentralElecPlots(this,"Central/", "Central"),
										      m_oFrwdElecPlots(this, "Frwd/", "Forward"),
										      m_oTruthIsoPlots(this, "Truth/Iso/", "Truth Electron Prompt"),
										      author(nullptr),
										      res_et(nullptr),
										      res_eta(nullptr),
										      res_et_cut(nullptr),
										      res_eta_cut(nullptr),
                                              res_et_cut_pt_20(nullptr),
                                              res_eta_cut_pt_20(nullptr),
										      //pt_ratio(nullptr),
										      matrix(nullptr)

{}	

void ElectronValidationPlots::initializePlots(){

  author       = Book1D("author", "Electron  Author ; author;Events", 20, -0.5, 19.5);
  res_et       = BookTProfile("res_et"," IsoElectron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta      = BookTProfile("res_eta"," IsoElectron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  res_et_cut   = BookTProfile("res_et_cut"," IsoElectron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta_cut  = BookTProfile("res_eta_cut"," IsoElectron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  res_et_cut_pt_20   = BookTProfile("res_et_cut_pt_20"," Prompt Electron;E_{T}^{Truth}, [GeV];(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",100, 0., 200.);
  res_eta_cut_pt_20  = BookTProfile("res_eta_cut_pt_20"," Prompt Electron;#eta;(E_{T} - E_{T}^{Truth})/E_{T}^{Truth}",50, -2.5, 2.5);
  matrix       = Book2D("matrix","reco vs truth pt",200,0.,200.,200,0.,200.);

}
 
void ElectronValidationPlots::fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt) {

  float weight = 1.;
  weight = eventInfo.beamSpotWeight();

  author->Fill(electron.author(),weight);
  
  if(electron.author()&xAOD::EgammaParameters::AuthorElectron||
     electron.author(xAOD::EgammaParameters::AuthorAmbiguous))  {
    m_oCentralElecPlots.fill(electron, eventInfo, isPrompt);
  } 
  else if(electron.author()&xAOD::EgammaParameters::AuthorFwdElectron) {
    m_oFrwdElecPlots.fill(electron, eventInfo, isPrompt);
  } 
}
