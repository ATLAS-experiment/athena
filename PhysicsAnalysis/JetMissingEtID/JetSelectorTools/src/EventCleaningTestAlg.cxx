/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM includes

// Local includes
#include "EventCleaningTestAlg.h"
#include "StoreGate/WriteDecorHandle.h"


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
EventCleaningTestAlg::EventCleaningTestAlg(const std::string& name,
                                             ISvcLocator* svcLoc)
    : AthAlgorithm(name, svcLoc) {}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
StatusCode EventCleaningTestAlg::initialize()
{
  ATH_MSG_INFO("Initialize");

  // Try to retrieve the tool
  ATH_CHECK( m_ecTool.retrieve() );
  ATH_CHECK( m_jetKey.initialize());
  ATH_CHECK( m_evtKey.initialize() );
  // Create the decorator
  // Use an if statement to leave a legacy 
  std::string labelString = m_cleaningLevel;
  if (m_jetKey.key() == "AntiKt4EMTopoJets") {
    labelString = m_cleaningLevel + "_EMTopo";
  }
  else {}

  m_evtInfoDecor = m_evtKey.key() + "." + m_prefix + "eventClean_"+labelString;
  ATH_CHECK(m_evtInfoDecor.initialize(m_doEvent));
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------
StatusCode EventCleaningTestAlg::execute()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Jets
  SG::ReadHandle<xAOD::JetContainer> jets{m_jetKey, ctx};
  if (!jets.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve jet collection "<<m_jetKey.fullKey());
    return StatusCode::FAILURE;
  }
  // Apply the event cleaning
  bool result = m_ecTool->acceptEvent(jets.cptr());

  //Decorate event
  if(m_doEvent){
    SG::WriteDecorHandle<xAOD::EventInfo, char> eventInfo{m_evtInfoDecor, ctx};
    if (!eventInfo.isValid()){
       ATH_MSG_FATAL("Failed to retrieve the event info "<<m_evtKey.fullKey());
       return StatusCode::FAILURE;
    }
    eventInfo(*eventInfo) = result;
  }

  return StatusCode::SUCCESS;
}

