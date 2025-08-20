/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJetConditionConfig_uht1tau.h"
#include "GaudiKernel/StatusCode.h"
#include "./UHT1TauCondition.h"
#include "./ArgStrToDouble.h"

TrigJetConditionConfig_uht1tau::TrigJetConditionConfig_uht1tau(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent){
}


StatusCode TrigJetConditionConfig_uht1tau::initialize() {
  CHECK(checkVals());
  
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_uht1tau::getCondition() const {
  auto a2d = ArgStrToDouble();
  return std::make_unique<UHT1TauCondition>(
    a2d(m_min),
    m_name_ptau,
    m_name_pu,
    m_name_valid);
}


StatusCode TrigJetConditionConfig_uht1tau::checkVals() const {
  return StatusCode::SUCCESS;
}
