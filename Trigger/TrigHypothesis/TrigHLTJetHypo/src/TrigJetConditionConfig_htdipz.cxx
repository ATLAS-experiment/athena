/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for ht+dipz Conditions
 */
#include "TrigJetConditionConfig_htdipz.h"
#include "GaudiKernel/StatusCode.h"
#include "./DipzLikelihoodCmp.h"
#include "./MaxCombinationCondition.h"
#include "./HTConditionFastReduction.h"
#include "./ArgStrToDouble.h"


TrigJetConditionConfig_htdipz::TrigJetConditionConfig_htdipz(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent) :
  base_class(type, name, parent){

}


StatusCode TrigJetConditionConfig_htdipz::initialize() {
  CHECK(checkVals());

  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_htdipz::getCondition() const {
  auto a2d = ArgStrToDouble();

  DipzLikelihoodCmp mlplComp(m_decName_z, m_decName_negLogSigma2);  
  std::unique_ptr<HTConditionFastReduction> HTcond = std::make_unique<HTConditionFastReduction>(a2d(m_min), a2d(m_max));
  return std::make_unique<MaxCombinationCondition<DipzLikelihoodCmp>>(a2d(m_capacity), std::move(HTcond), mlplComp);

}


StatusCode TrigJetConditionConfig_htdipz::checkVals() const {
  auto a2d = ArgStrToDouble();
  if (a2d(m_min) > a2d(m_max)){
    ATH_MSG_ERROR("htMin > htMax");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}