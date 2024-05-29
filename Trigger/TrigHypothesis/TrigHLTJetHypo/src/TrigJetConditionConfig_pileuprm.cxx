/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for PT Condition
 */
#include "TrigJetConditionConfig_pileuprm.h"
#include "GaudiKernel/StatusCode.h"
#include "./PileupRemovalCondition.h"
#include "./ArgStrToDouble.h"


TrigJetConditionConfig_pileuprm::TrigJetConditionConfig_pileuprm(const std::string& type,
						     const std::string& name,
						     const IInterface* parent) :
  base_class(type, name, parent){
  
}


StatusCode TrigJetConditionConfig_pileuprm::initialize() {
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_pileuprm::getCondition() const {
  auto a2d = ArgStrToDouble();
 
  return std::make_unique<PileupRemovalCondition>(a2d(m_min),
               a2d(m_max));
}
				     

StatusCode TrigJetConditionConfig_pileuprm::checkVals() const {
  auto a2d = ArgStrToDouble();
  if (a2d(m_min) > a2d(m_max)){
    ATH_MSG_ERROR(" min LogR cut >  max LogR cut");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
