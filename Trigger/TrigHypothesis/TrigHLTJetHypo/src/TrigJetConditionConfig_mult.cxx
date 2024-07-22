/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for Mult Conditions
 */
#include "TrigJetConditionConfig_mult.h"
#include "GaudiKernel/StatusCode.h"
#include "./MultiplicityCondition.h"
#include "./ArgStrToSizeT.h"

TrigJetConditionConfig_mult::TrigJetConditionConfig_mult(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent) :
  base_class(type, name, parent){

}


StatusCode TrigJetConditionConfig_mult::initialize() {
  
  auto cnvtr = ArgStrToSizeT();
  m_min_szt = cnvtr(m_min);
  m_max_szt = cnvtr(m_max);

  CHECK(checkVals());

  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_mult::getCondition() const {
  return std::make_unique<MultiplicityCondition>(m_min_szt, m_max_szt);
}


StatusCode TrigJetConditionConfig_mult::checkVals() const {
  if (m_min_szt >= m_max_szt){
    ATH_MSG_ERROR("multMin >= multMax " << m_min_szt << " " << m_max_szt
		  << " inputs " << m_min << " " << m_max);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
