/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for dipz Conditions
 */
#include "TrigJetConditionConfig_dipzmlpl.h"
#include "GaudiKernel/StatusCode.h"
#include "./DipzMLPLCondition.h"
#include "./ArgStrToDouble.h"

TrigJetConditionConfig_dipzmlpl::TrigJetConditionConfig_dipzmlpl(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent) :
  base_class(type, name, parent){

}


StatusCode TrigJetConditionConfig_dipzmlpl::initialize() {
  CHECK(checkVals());

  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_dipzmlpl::getCondition() const {
  auto a2d = ArgStrToDouble();
  return std::make_unique<DipzMLPLCondition>(
    a2d(m_min),
    a2d(m_capacity), 
    m_decName_z,
    m_decName_sigma,
    m_sigmaIsStdDev);
}


StatusCode TrigJetConditionConfig_dipzmlpl::checkVals() const {
  return StatusCode::SUCCESS;
}
