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
  // m_evtInfoHSTPDecor = m_evtKey.key() + "." + m_prefix + "eventClean_HSTP";
  ATH_CHECK(m_evtInfoDecor.initialize(m_doEvent));
  // ATH_CHECK(m_evtInfoHSTPDecor.initialize(m_doHSTPFiltering));
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------
StatusCode EventCleaningTestAlg::execute(const EventContext& ctx)
{
  // Jets
  SG::ReadHandle<xAOD::JetContainer> jets{m_jetKey, ctx};
  if (!jets.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve jet collection " << m_jetKey.fullKey());
    return StatusCode::FAILURE;
  }

  // EventInfo
  SG::ReadHandle<xAOD::EventInfo> eventInfo{m_evtKey, ctx};
  if (!eventInfo.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve the event info " << m_evtKey.fullKey());
    return StatusCode::FAILURE;
  }

  // Apply the event cleaning
  const bool result = m_ecTool->acceptEvent(jets.cptr());

  //Decorate event
  if(m_doEvent){
    SG::WriteDecorHandle<xAOD::EventInfo, char> eventDecor{m_evtInfoDecor, ctx};
    if (!eventDecor.isValid()){
       ATH_MSG_FATAL("Failed to retrieve the event info "<<m_evtKey.fullKey());
       return StatusCode::FAILURE;
    }
    eventDecor(*eventInfo) = result;
  }

  //Decorate event with HSTP if needed

  // if (m_doHSTPFiltering){
  //   const xAOD::JetContainer* tjets   = nullptr;
  //   const xAOD::JetContainer* tPUjets = nullptr;
    
  //   SG::ReadHandle<xAOD::JetContainer> truthJets{ "AntiKt4TruthDressedWZJets", ctx};
  //   if (truthJets.isValid()) tjets = truthJets.cptr();
  //   else {
  //     SG::ReadHandle<xAOD::JetContainer> truthJetsWZ{"AntiKt4TruthWZJets", ctx};
  //     if (truthJetsWZ.isValid())  tjets = truthJetsWZ.cptr();
  //     else { 
  //       SG::ReadHandle<xAOD::JetContainer> truthJetsDefault{ "AntiKt4TruthJets", ctx};
  //       if (truthJetsDefault.isValid()) tjets = truthJetsDefault.cptr();
  //     }
  //   }

  //   if (!tjets) {
  //     ATH_MSG_FATAL("Failed to retrieve truth jet collection");
  //     return StatusCode::FAILURE;
  //   }

  //   SG::ReadHandle<xAOD::JetContainer> truthPUJets{"InTimeAntiKt4TruthJets", ctx};
  //   if (truthPUJets.isValid()) {
  //     tPUjets = truthPUJets.cptr();
  //   } else {
  //     ATH_MSG_FATAL("Failed to retrieve truth Pile-Up jet collection");
  //     return StatusCode::FAILURE;
  //   }

  //   SG::WriteDecorHandle<xAOD::EventInfo, char> eventInfoHSTP{ m_evtInfoHSTPDecor, ctx};
    
  //   const bool hstpResult = m_ecTool->passHSTPFilter(tjets, tPUjets, 5000);
  //     if (!eventInfoHSTP.isValid()) { 
  //       ATH_MSG_FATAL("Failed to create event decoration " << m_evtInfoHSTPDecor.key());
  //       return StatusCode::FAILURE;
  //   }
  //   eventInfoHSTP(*eventInfo) = hstpResult;
  // }

  return StatusCode::SUCCESS;
}




