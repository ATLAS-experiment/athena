/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauPi0ScoreCalculator.h"
#include "tauRecTools/HelperFunctions.h"
#include "xAODPFlow/PFO.h"


TauPi0ScoreCalculator::TauPi0ScoreCalculator(const std::string& name) :
    TauRecToolBase(name) {
}


StatusCode TauPi0ScoreCalculator::initialize() {
  std::string weightFile = find_file(m_weightfile);

  m_mvaBDT = std::make_unique<tauRecTools::BDTHelper>();
  ATH_CHECK(m_mvaBDT->initialize(weightFile));
 
  return StatusCode::SUCCESS;
}


StatusCode TauPi0ScoreCalculator::executeTool(xAOD::TauJet& pTau,
					      const EventContext& /*ctx*/,
					      xAOD::PFOContainer& neutralPFOContainer) const {
  // Only run on 0-5 prong taus 
  if (!tauRecTools::doPi0andShots(pTau)) {
    return StatusCode::SUCCESS;
  }

  // retrieve neutral PFOs from tau, calculate BDT scores and store them in PFO
  for(size_t i=0; i<pTau.nProtoNeutralPFOs(); i++) {
    xAOD::PFO* neutralPFO = neutralPFOContainer.at( pTau.protoNeutralPFO(i)->index() );
    float BDTScore = calculateScore(neutralPFO);
    neutralPFO->setBDTPi0Score(BDTScore);
  }

  return StatusCode::SUCCESS;
}


float TauPi0ScoreCalculator::calculateScore(const xAOD::PFO* neutralPFO) const {
  
  std::map<TString, float> availableVariables;

  int ivariable = 0;  
  float fvariable = 0.;

  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_FIRST_ETA, fvariable) == false) {
    ATH_MSG_WARNING("Can't find FIRST_ETA. Set it to 0.");
  }
  fvariable = std::abs(fvariable);
  availableVariables.insert(std::make_pair("Pi0Cluster_Abs_FIRST_ETA", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_SECOND_R, fvariable) == false) {
    ATH_MSG_WARNING("Can't find SECOND_R. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_SECOND_R", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_DELTA_THETA, fvariable) == false) {
    ATH_MSG_WARNING("Can't find DELTA_THETA. Set it to 0.");
  }
  fvariable = std::abs(fvariable);
  availableVariables.insert(std::make_pair("Pi0Cluster_Abs_DELTA_THETA", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_CENTER_LAMBDA, fvariable) == false) {
    ATH_MSG_WARNING("Can't find CENTER_LAMBDA. Set it to 0.");
  }
  fvariable = fmin(fvariable, 1000.);
  availableVariables.insert(std::make_pair("Pi0Cluster_CENTER_LAMBDA_helped", fvariable));
  
  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_LONGITUDINAL, fvariable) == false) {
    ATH_MSG_WARNING("Can't find LONGITUDINAL. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_LONGITUDINAL", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_ENG_FRAC_EM, fvariable) == false) {
    ATH_MSG_WARNING("Can't find ENG_FRAC_EM. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_ENG_FRAC_EM", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_ENG_FRAC_CORE, fvariable) == false) { 
    ATH_MSG_WARNING("Can't find ENG_FRAC_CORE. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_ENG_FRAC_CORE", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_SECOND_ENG_DENS, fvariable) == false) { 
    ATH_MSG_WARNING("Can't find SECOND_ENG_DENS. Set it to 0.");
  }
  if(fvariable==0.) {
    fvariable=-50.;
  }
  else {
    fvariable = log(fvariable);
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_log_SECOND_ENG_DENS", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_EM1CoreFrac, fvariable) == false) { 
    ATH_MSG_WARNING("Can't find EM1CoreFrac. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_EcoreOverEEM1", fvariable));
  
  ivariable = 0;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_NPosECells_EM1, ivariable) == false) { 
    ATH_MSG_WARNING("Can't find NPosECells_EM1. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_NPosECells_EM1", static_cast<float>(ivariable)));

  ivariable = 0;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_NPosECells_EM2, ivariable) == false) { 
    ATH_MSG_WARNING("Can't find NPosECells_EM2. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_NPosECells_EM2", static_cast<float>(ivariable)));
  
  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM1, fvariable) == false) { 
    ATH_MSG_WARNING("Can't find firstEtaWRTClusterPosition_EM1. Set it to 0.");
  }
  fvariable = std::abs(fvariable);
  availableVariables.insert(std::make_pair("Pi0Cluster_AbsFirstEtaWRTClusterPosition_EM1", fvariable));

  fvariable = 0.;
  if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM2, fvariable) == false) { 
    ATH_MSG_WARNING("Can't find secondEtaWRTClusterPosition_EM2. Set it to 0.");
  }
  availableVariables.insert(std::make_pair("Pi0Cluster_secondEtaWRTClusterPosition_EM2", fvariable)); 

  // Calculate BDT score, will be -999 when availableVariables lack variables
  float score = m_mvaBDT->getGradBoostMVA(availableVariables);

  return score;
}
