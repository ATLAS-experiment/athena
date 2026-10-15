/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"
#include "TauAnalysisTools/TauResolutionTool.h"

#include "TFile.h"

#include <AsgTools/AsgToolConfig.h>

namespace TauAnalysisTools
{

TauResolutionTool::TauResolutionTool( const std::string& sName )
  : asg::AsgMetadataTool( sName )
{}

TauResolutionTool::~TauResolutionTool()
{}

StatusCode TauResolutionTool::initialize()
{
  ATH_MSG_INFO( "Initializing TauResolutionTool" );

 
  std::string calFilePath = PathResolverFindCalibFile(m_calFileName);
  std::unique_ptr<TFile> calFile(TFile::Open(calFilePath.c_str(), "READ"));
  ATH_MSG_INFO("Using calibration file: " << calFilePath);

  // 2D array with (eta, decay mode) as index
  TGraph* graph = nullptr;
  std::string graphName="";
  for (size_t decayModeIndex = 0; decayModeIndex < DecayModeBinning; ++decayModeIndex) {
    for (size_t etaIndex = 0; etaIndex < EtaBinning; ++etaIndex) {

      // Calo TES: resolution
      graphName = "tauRec/Graph_from_ResolutionEt_tauRec_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames[etaIndex];
      graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
      if(graph){
        m_caloResMaxEt[decayModeIndex][etaIndex] = TMath::MaxElement(graph->GetN(), graph->GetX());
        m_caloRes[decayModeIndex][etaIndex] = std::unique_ptr<TGraph>(graph);
        ATH_MSG_DEBUG("Adding graph: " << graphName);
      }
      else {
        ATH_MSG_FATAL("Failed to get an object with name " << graphName);
        return StatusCode::FAILURE;
      }
   
      graphName = "FinalCalib/Graph_from_ResolutionEt_FinalCalib_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames.at(etaIndex);
      graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
      if(graph){
        m_mvaResMaxEt.at(decayModeIndex).at(etaIndex) = TMath::MaxElement(graph->GetN(), graph->GetX());
	m_mvaRes.at(decayModeIndex).at(etaIndex) = std::unique_ptr<TGraph>(graph);
	ATH_MSG_DEBUG("Adding graph: " << graphName);
      }  
      else {
	ATH_MSG_FATAL("Failed to get an object with name " << graphName);
	return StatusCode::FAILURE;
      }
    }
  }
  calFile->Close();

  return StatusCode::SUCCESS;
}


bool TauResolutionTool::isValid(const xAOD::TauJet& tau) const {
  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau);
  if (decayMode < xAOD::TauJetParameters::Mode_1p0n || decayMode > xAOD::TauJetParameters::Mode_3pXn) {
    ATH_MSG_DEBUG("Decay mode is not supported !");
    return false;
  }

  int etaIndex = getEtaIndex(tau.etaTauEnergyScale());
  if (etaIndex > 4) {
    ATH_MSG_DEBUG("Eta is out of the supported range !");
    return false;
  }

  return true;
}

int TauResolutionTool::getEtaIndex(float eta) const {
  float abseta = std::abs(eta);
      
  if (abseta < 0.3) {
    return 0;
  }
  if (abseta < 0.8) {
    return 1;
  }
  if (abseta < 1.3) {
    return 2;
  }
  if (abseta < 1.6) {
    return 3;
  }
  if (abseta < 2.6) {
    return 4;
  }

  return 99;
}

xAOD::TauJetParameters::DecayMode TauResolutionTool::getDecayMode(const xAOD::TauJet& tau) const {
  int decayMode = xAOD::TauJetParameters::DecayMode::Mode_Error;
  tau.panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayMode, decayMode);

  return static_cast<xAOD::TauJetParameters::DecayMode>(decayMode);
}

int TauResolutionTool::getDecayModeIndex(xAOD::TauJetParameters::DecayMode decayMode) const {
  return static_cast<int>(decayMode);
}

double TauResolutionTool::getMvaEnergyResolution(const xAOD::TauJet& tau) const {

  // Assume the resolution to be 100% when no parametrisation is available
  if (!isValid(tau)) return 1.0;
  
  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau);
  int decayModeIndex = getDecayModeIndex(decayMode);

  int etaIndex = getEtaIndex(tau.etaFinalCalib());

  double pt = std::min(tau.ptFinalCalib(), m_mvaResMaxEt.at(decayModeIndex).at(etaIndex));
  double resolution = m_mvaRes.at(decayModeIndex).at(etaIndex)->Eval(pt);

  return resolution;
}

double TauResolutionTool::getCaloResolution(double et, int decayModeIndex, int etaIndex) const {
  double x = std::min(et, m_caloResMaxEt.at(decayModeIndex).at(etaIndex));
  double resolution = m_caloRes.at(decayModeIndex).at(etaIndex)->Eval(x);

  return resolution;
}

bool TauResolutionTool::getTESCompatibility(const xAOD::TauJet& tau) const {
  if (! isValid(tau)) return false;

  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau); //can return 7
  int decayModeIndex = getDecayModeIndex(decayMode);

  int etaIndex = getEtaIndex(tau.etaTauEnergyScale());

  double caloSigma = tau.ptTauEnergyScale() * getCaloResolution(tau.ptTauEnergyScale(), decayModeIndex, etaIndex);
  double deltaEt = tau.ptFinalCalib() - tau.ptTauEnergyScale();

  bool compatibility  = true;

  if (std::abs(deltaEt) > 5 * caloSigma) {
    compatibility = false;
  }

  return compatibility;
}

} // namespace TauAnalysisTools
