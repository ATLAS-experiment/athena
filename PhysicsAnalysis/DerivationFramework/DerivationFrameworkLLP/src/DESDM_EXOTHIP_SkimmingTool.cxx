/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DESDM_EXOTHIP_SkimmingTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Priyanka Kumari (pkumari@cern.ch)
// follows closely DerivationFrameworkExamples

#include "DerivationFrameworkLLP/DESDM_EXOTHIP_SkimmingTool.h"
#include "xAODTrigRinger/TrigRNNOutputContainer.h"
#include "xAODTrigRinger/TrigRNNOutput.h"

#include <vector>
#include <string>
#include <algorithm>

// Constructor

DerivationFramework::DESDM_EXOTHIP_SkimmingTool::DESDM_EXOTHIP_SkimmingTool(const std::string& CPP_exothip,
									    const std::string& skimtool,
									    const IInterface* interface ) :
  
  AthAlgTool(CPP_exothip, skimtool, interface),
  m_trigDec("Trig::TrigDecisionTool/TrigDecisionTool")

{
  declareInterface<DerivationFramework::ISkimmingTool>(this);
  declareProperty("HTTRTHitsCounter", m_trnnoutContName);
  declareProperty("MinHTRatioWedge", m_minHTratioWedge);
}

StatusCode DerivationFramework::DESDM_EXOTHIP_SkimmingTool::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");
  ATH_CHECK(m_trigDec.retrieve());
  
  ATH_MSG_INFO("Retrieved tool: " << m_trigDec);
  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::DESDM_EXOTHIP_SkimmingTool::finalize() {
  
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed " << m_ntot << " events, " << m_npass << " events passed filter ");
  
  return StatusCode::SUCCESS;
}

bool DerivationFramework::DESDM_EXOTHIP_SkimmingTool::eventPassesFilter() const{
  
  m_ntot++;
  
  auto trig_fht_wedge = std::make_unique<std::vector<float>>();

  const xAOD::TrigRNNOutputContainer* trnnoutCont = 0;
  StatusCode sc = evtStore()->retrieve( trnnoutCont, m_trnnoutContName);
  if (sc.isFailure()){
    ATH_MSG_FATAL("No HTTRT collection with name " << m_trnnoutContName << " found in StoreGate!");
    return false;
  }
   
  for (const xAOD::TrigRNNOutput* trnnout : *trnnoutCont){
    trig_fht_wedge->push_back(trnnout->rnnDecision().at(3));
  }

  for (float value : *trig_fht_wedge) {
    if (static_cast<double>(value) > m_minHTratioWedge) { 
      m_npass++;
      return true;
    }
  }

  return false;
}
   
 
  



