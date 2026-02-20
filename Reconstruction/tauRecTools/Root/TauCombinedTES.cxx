/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauCombinedTES.h"

#include "TFile.h"

#include <cmath>



TauCombinedTES::TauCombinedTES(const std::string& name) :
  TauRecToolBase(name) {
}



StatusCode TauCombinedTES::initialize() {

  std::string calFilePath = find_file(m_calFileName);
  std::unique_ptr<TFile> calFile(TFile::Open(calFilePath.c_str(), "READ"));
  ATH_MSG_INFO("Using calibration file: " << calFilePath);

  // 1D array with decay mode as index
  TH1F* hist = nullptr;
  TF1* tf1 = nullptr;
  std::string histName = "";
  for (size_t decayModeIndex = 0; decayModeIndex < DecayModeBinning; ++ decayModeIndex) {
    histName = "CorrelationCoeff_tauRec_" + m_decayModeNames[decayModeIndex];
    hist = dynamic_cast<TH1F*> (calFile->Get(histName.c_str()));
    if(hist) {
      hist->SetDirectory(nullptr);
      m_correlationHists[decayModeIndex] = std::unique_ptr<TH1F>(hist);
      ATH_MSG_DEBUG("Adding corr hist: " << histName);
    }
    else {
      ATH_MSG_FATAL("Failed to get an object with name " << histName);
      return StatusCode::FAILURE;
    }

    histName = "nSigmaCompatibility_" + m_decayModeNames[decayModeIndex];
    tf1 = dynamic_cast<TF1*> (calFile->Get(histName.c_str()));
    if(tf1) {
      m_nSigmaCompatibility[decayModeIndex] = std::unique_ptr<TF1>(tf1);
      ATH_MSG_DEBUG("Adding compatibility TF1: " << histName);
    }
    else {
      ATH_MSG_FATAL("Failed to get an object with name " << histName);
      return StatusCode::FAILURE;
    }
  }

  // 2D array with (eta, decay mode) as index
  TGraph* graph = nullptr;
  std::string graphName="";
  for (size_t decayModeIndex = 0; decayModeIndex < DecayModeBinning; ++decayModeIndex) {
    for (size_t etaIndex = 0; etaIndex < EtaBinning; ++etaIndex) {
      // Calo TES: relative bias
      graphName = "tauRec/Graph_from_MeanEt_tauRec_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames[etaIndex];
      graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
      if(graph) {
        m_caloRelBiasMaxEt[decayModeIndex][etaIndex] = TMath::MaxElement(graph->GetN(), graph->GetX());
        m_caloRelBias[decayModeIndex][etaIndex] = std::unique_ptr<TGraph>(graph);
        ATH_MSG_DEBUG("Adding graph: " << graphName);
      }
      else {
       	ATH_MSG_FATAL("Failed to get an object with name " << graphName);
       	return StatusCode::FAILURE;
      }

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

      // PanTau: relative bias
      graphName = "ConstituentEt/Graph_from_MeanEt_ConstituentEt_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames[etaIndex];
      graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
      if(graph){
        m_panTauRelBiasMaxEt[decayModeIndex][etaIndex] = TMath::MaxElement(graph->GetN(), graph->GetX());
        m_panTauRelBias[decayModeIndex][etaIndex] = std::unique_ptr<TGraph>(graph);
        ATH_MSG_DEBUG("Adding graph: " << graphName);
      }
      else {
       	ATH_MSG_FATAL("Failed to get an object with name " << graphName);
       	return StatusCode::FAILURE;
      }

      // PanTau: resolution
      graphName = "ConstituentEt/Graph_from_ResolutionEt_ConstituentEt_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames[etaIndex];
      graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
      if(graph){
        m_panTauResMaxEt[decayModeIndex][etaIndex] = TMath::MaxElement(graph->GetN(), graph->GetX());
        m_panTauRes[decayModeIndex][etaIndex] = std::unique_ptr<TGraph>(graph);
        ATH_MSG_DEBUG("Adding graph: " << graphName);
      }
      else {
        ATH_MSG_FATAL("Failed to get an object with name " << graphName);
       	return StatusCode::FAILURE;
      }

      // MVA resolution, optional
      if(m_useMvaResolution) {
	graphName = "FinalCalib/Graph_from_ResolutionEt_FinalCalib_" + m_decayModeNames[decayModeIndex] + "_" + m_etaBinNames[etaIndex];
	graph = dynamic_cast<TGraph*> (calFile->Get(graphName.c_str()));
	if(graph){
	  m_mvaResMaxEt[decayModeIndex][etaIndex] = TMath::MaxElement(graph->GetN(), graph->GetX());
	  m_mvaRes[decayModeIndex][etaIndex] = std::unique_ptr<TGraph>(graph);
	  ATH_MSG_DEBUG("Adding graph: " << graphName);
	}
	else {
	  ATH_MSG_FATAL("Failed to get an object with name " << graphName);
	  return StatusCode::FAILURE;
	}
      }
    }
  }
  calFile->Close();

  return StatusCode::SUCCESS;
}



StatusCode TauCombinedTES::execute(xAOD::TauJet& tau) const {
  TLorentzVector combinedP4(tau.p4(xAOD::TauJetParameters::TauEnergyScale));

  // Parameterization is only valid for |eta| < 2.5, and decay modes of 1p0n, 1p1n, 1pXn, 3p0n, 3pXn
  // If these variables of the given tau candidate are outside the range, we just use calo TES
  if(isValid(tau)) {
    combinedP4 = getCombinedP4(tau);
  }

  static const SG::Accessor<float> decPtCombined("ptCombined");
  static const SG::Accessor<float> decEtaCombined("etaCombined");
  static const SG::Accessor<float> decPhiCombined("phiCombined");
  static const SG::Accessor<float> decMCombined("mCombined");

  decPtCombined(tau) = combinedP4.Pt();
  decEtaCombined(tau) = combinedP4.Eta();
  decPhiCombined(tau) = combinedP4.Phi();
  decMCombined(tau) = combinedP4.M();

  return StatusCode::SUCCESS;
}



bool TauCombinedTES::getTESCompatibility(const xAOD::TauJet& tau) const {
  if (! isValid(tau)) return false;

  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau);
  int decayModeIndex = getDecayModeIndex(decayMode);

  int etaIndex = getEtaIndex(tau.etaTauEnergyScale());

  double caloSigma = tau.ptTauEnergyScale() * getCaloResolution(tau.ptTauEnergyScale(), decayModeIndex, etaIndex);
  double deltaEt = tau.ptFinalCalib() - tau.ptTauEnergyScale();

  bool compatibility  = true;

  // FIXME: should we use combinedSigma here ??
  if (std::abs(deltaEt) > 5 * caloSigma) {
    compatibility = false;
  }

  return compatibility;
}



int TauCombinedTES::getEtaIndex(float eta) const {
  // It would be better to retrieve eta bins from the calibration file, e.g. for upgrade studies!
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
  // slightly extend the tau eta range, as |eta|<2.5 applies to the seed jet
  if (abseta < 2.6) {
    return 4;
  }

  return 99;
}



xAOD::TauJetParameters::DecayMode TauCombinedTES::getDecayMode(const xAOD::TauJet& tau) const {
  int decayMode = xAOD::TauJetParameters::DecayMode::Mode_Error;
  tau.panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayMode, decayMode);

  return static_cast<xAOD::TauJetParameters::DecayMode>(decayMode);
}



int TauCombinedTES::getDecayModeIndex(xAOD::TauJetParameters::DecayMode decayMode) const {
  return static_cast<int>(decayMode);
}



bool TauCombinedTES::isValid(const xAOD::TauJet& tau) const {
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



double TauCombinedTES::getCorrelation(int decayModeIndex, int etaIndex) const {
  return m_correlationHists[decayModeIndex]->GetBinContent(etaIndex);
}



double TauCombinedTES::getCaloCalEt(double caloEt,
				    int decayModeIndex,
				    int etaIndex) const {
  // ratio stored in the calibration graph equals (caloEt-truthEt)/caloEt
  double ratio = 0.0;

  // FIXME: If caloEt is larger than max et, could we use the ratio at
  // max et, instead of setting it to zero
  if (caloEt <= m_caloRelBiasMaxEt[decayModeIndex][etaIndex]) {
    ratio = m_caloRelBias[decayModeIndex][etaIndex]->Eval(caloEt);
  }

  double caloCalEt = caloEt - ratio * caloEt;

  return caloCalEt;
}



double TauCombinedTES::getPanTauCalEt(double panTauEt,
				      int decayModeIndex,
				      int etaIndex) const {
  // ratio stored in the calibration graph equals (panTauEt-truthEt)/panTauEt
  double ratio = 0.0;

  // Substructure is badly determined at high pt, as track momentum is pooryly measured
  if (panTauEt <= m_panTauRelBiasMaxEt[decayModeIndex][etaIndex]) {
    ratio = m_panTauRelBias[decayModeIndex][etaIndex]->Eval(panTauEt);
  }

  double panTauCalEt = panTauEt - ratio * panTauEt;

  return panTauCalEt;
}



double TauCombinedTES::getMvaEnergyResolution(const xAOD::TauJet& tau) const {
  // Assume the resolution to be 100% when no parametrisation is available
  // "validity" criteria might have to be revised if such "invalid taus" end up in analyses
  if (!isValid(tau) || !m_useMvaResolution) return 1.0;

  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau);
  int decayModeIndex = getDecayModeIndex(decayMode);

  int etaIndex = getEtaIndex(tau.etaFinalCalib());

  double pt = std::min(tau.ptFinalCalib(), m_mvaResMaxEt[decayModeIndex][etaIndex]);
  double resolution = m_mvaRes[decayModeIndex][etaIndex]->Eval(pt);

  return resolution;
}



double TauCombinedTES::getCaloResolution(double et, int decayModeIndex, int etaIndex) const {
  double x = std::min(et, m_caloResMaxEt[decayModeIndex][etaIndex]);
  double resolution = m_caloRes[decayModeIndex][etaIndex]->Eval(x);

  return resolution;
}



double TauCombinedTES::getPanTauResolution(double et, int decayModeIndex, int etaIndex) const {
  double x = std::min(et, m_panTauResMaxEt[decayModeIndex][etaIndex]);
  double resolution = m_panTauRes[decayModeIndex][etaIndex]->Eval(x);

  return resolution;
}



double TauCombinedTES::getWeight(double caloSigma,
				 double panTauSigma,
				 double correlation) const {
  double cov = correlation * caloSigma * panTauSigma;
  double caloWeight = std::pow(panTauSigma, 2) - cov;
  double panTauWeight = std::pow(caloSigma, 2) - cov;
  
  double weight = (caloWeight + panTauWeight !=0.) ? caloWeight/(caloWeight + panTauWeight) : 0.;
  // enforce that the weight is within [0,1]
  return std::clamp(weight, 0., 1.);
}

double TauCombinedTES::getCompatibilitySigma(double caloSigma,
					     double panTauSigma,
					     double correlation) const {
  double compatibilitySigma2 = std::pow(caloSigma, 2) + std::pow(panTauSigma, 2) - 2 * correlation * caloSigma * panTauSigma;

  return std::sqrt(compatibilitySigma2);
}



double TauCombinedTES::getNsigmaCompatibility(double et, int decayModeIndex) const {
  double nsigma = m_nSigmaCompatibility.at(decayModeIndex)->Eval(et);

  if (nsigma < 0.) return 0.;

  return nsigma;
}



double TauCombinedTES::getCombinedEt(double caloEt,
				     double panTauEt,
				     xAOD::TauJetParameters::DecayMode decayMode,
				     float eta) const {
  // Obtain the index of calibration graph
  int decayModeIndex = getDecayModeIndex(decayMode);
  int etaIndex = getEtaIndex(eta);

  // Obtain the calibration parameter based on the index
  // -- Correlation between calo TES and PanTau
  double correlation = getCorrelation(decayModeIndex, etaIndex);

  // -- Sigma of the difference between reconstruted et and truth et at calo TES
  double caloSigma = caloEt * getCaloResolution(caloEt, decayModeIndex, etaIndex);
  if (0. == caloSigma) {
    ATH_MSG_WARNING("Calo TES: Et resolution at " << caloEt << " is 0");
    m_caloRes[decayModeIndex][etaIndex]->Print("all");
    return 0.;
  }

  // -- Sigma of the difference between reconstruted et and truth et at PanTau
  double panTauSigma = panTauEt * getPanTauResolution(panTauEt, decayModeIndex, etaIndex);
  if (0. == panTauSigma) {
    ATH_MSG_WARNING("PanTau: Et resolution at " << panTauEt << " is 0");
    m_panTauRes[decayModeIndex][etaIndex]->Print("all");
    return 0.;
  }

  // -- Et at calo TES with bias corrected
  double caloCalEt = getCaloCalEt(caloEt, decayModeIndex, etaIndex);

  // -- Et at PanTau with bias corrected
  double panTauCalEt = getPanTauCalEt(panTauEt, decayModeIndex, etaIndex);

  // Combination of calo TES and PanTau
  // FIXME: A more consistent way would be calculating the weight use bias corrected Et as input
  double weight = getWeight(caloSigma, panTauSigma, correlation);
  double weightedEt = weight * caloCalEt + (1 - weight) * panTauCalEt;
  double compatibilitySigma = getCompatibilitySigma(caloSigma, panTauSigma, correlation);

  // If the difference of calo TES and PanTau is too large, the combined result
  // may not be reliable
  // FIXME: A more consistent way would be calculating the NsigmaCompatibility use caloCalEt
  double deltaEt = caloCalEt - panTauCalEt;
  if (std::abs(deltaEt) > getNsigmaCompatibility(caloEt, decayModeIndex) * compatibilitySigma) {
    // FIXME: Why not use caloCalEt here ?
    weightedEt = caloEt;
  }

  ATH_MSG_DEBUG("Intermediate results\n" <<
                "coff: " << correlation << " sigma(calo): " << caloSigma << " sigma(constituent): " << panTauSigma <<
                "\ncalibrated et(calo): " << caloCalEt << " calibrated et(constituent): " << panTauCalEt <<
                "\nweight:" << weight << " combined et: " << weightedEt << " compatibility sigma: " << compatibilitySigma);

  return weightedEt;
}



TLorentzVector TauCombinedTES::getCombinedP4(const xAOD::TauJet& tau) const {
  TLorentzVector caloP4 = tau.p4(xAOD::TauJetParameters::TauEnergyScale);
  TLorentzVector panTauP4 = tau.p4(xAOD::TauJetParameters::PanTauCellBased);

  ATH_MSG_DEBUG("Four momentum at calo TES, pt: " << caloP4.Pt() << " eta: " << caloP4.Eta() <<
                " phi: " << caloP4.Phi() << " mass: " << caloP4.M());
  ATH_MSG_DEBUG("Four momentum at PanTau, pt: " << panTauP4.Pt() << " eta: " << panTauP4.Eta() <<
                " phi: " << panTauP4.Phi() << " mass: " << panTauP4.M());

  xAOD::TauJetParameters::DecayMode decayMode = getDecayMode(tau);

  double combinedEt = getCombinedEt(caloP4.Et(), panTauP4.Et(), decayMode, caloP4.Eta());

  // Et is the combination of calo TES and PanTau, but eta and phi is from PanTau
  TLorentzVector combinedP4;
  combinedP4.SetPtEtaPhiM(combinedEt, panTauP4.Eta(), panTauP4.Phi(), 0.);

  ATH_MSG_DEBUG("Combined four momentum, pt: " << combinedP4.Pt() << " eta: " << combinedP4.Eta() <<
                " phi: " << combinedP4.Phi() << " mass: " << combinedP4.M());

  return combinedP4;
}
