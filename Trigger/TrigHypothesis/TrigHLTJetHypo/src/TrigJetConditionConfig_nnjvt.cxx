/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for NNJVT Condition
 */
#include "TrigJetConditionConfig_nnjvt.h"
#include "GaudiKernel/StatusCode.h"
#include "./NNJVTCondition.h"

TrigJetConditionConfig_nnjvt::TrigJetConditionConfig_nnjvt(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent){
}


StatusCode TrigJetConditionConfig_nnjvt::initialize() {
  CHECK(checkVals());
  
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_nnjvt::getCondition() const {
  return std::make_unique<NNJVTCondition>(m_nnJvtName);
}

 
StatusCode TrigJetConditionConfig_nnjvt::checkVals() const {
  return StatusCode::SUCCESS;
}
