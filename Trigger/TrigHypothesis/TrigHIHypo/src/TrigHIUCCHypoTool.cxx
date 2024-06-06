/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigHIUCCHypoTool.h"
#include <GaudiKernel/StatusCode.h>

TrigHIUCCHypoTool::TrigHIUCCHypoTool(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent), 
  m_decisionId(HLT::Identifier::fromToolName(name))
{
}

const HLT::Identifier& TrigHIUCCHypoTool::getId() const {
  return m_decisionId;
}

StatusCode TrigHIUCCHypoTool::decide(const xAOD::HIEventShapeContainer* eventShapeContainer, bool& pass) const {
  ATH_MSG_DEBUG("Executing decide() of " << name());

  float totalFCalEt = 0;  
  for (const xAOD::HIEventShape* es: *eventShapeContainer) {
      const int layer = es->layer();
    if (layer < 21 or layer > 23) { //only use FCal information (calo samplings 21: FCAL0, 22: FCAL1, 23: FCAL2)
      continue;
    }
    totalFCalEt += es->et();
  }
  pass = totalFCalEt > m_FCalEtThreshold;
  return StatusCode::SUCCESS;
}