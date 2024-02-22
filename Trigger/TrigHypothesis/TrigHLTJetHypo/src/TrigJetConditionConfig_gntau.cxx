/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJetConditionConfig_gntau.h"
#include "GaudiKernel/StatusCode.h"
#include "./GNTauCondition.h"
#include "./ArgStrToDouble.h"

TrigJetConditionConfig_gntau::TrigJetConditionConfig_gntau(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent){
}


StatusCode TrigJetConditionConfig_gntau::initialize() {
  CHECK(checkVals());
  
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_gntau::getCondition() const {
  auto a2d = ArgStrToDouble();
  return std::make_unique<GNTauCondition>(
    a2d(m_min),
    m_name_ptau,
    m_name_pu,
    m_name_valid);
}


StatusCode TrigJetConditionConfig_gntau::checkVals() const {
  return StatusCode::SUCCESS;
}
