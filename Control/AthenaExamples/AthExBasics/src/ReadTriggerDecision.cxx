/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ReadTriggerDecision.h"

ReadTriggerDecision::ReadTriggerDecision(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {
}

// Initialise: run once at the start of the event
StatusCode ReadTriggerDecision::initialize() {

    // Initialise the tool handle
    ATH_CHECK(m_trigDec.retrieve());

    // Initialise counter  
    m_triggerCounter = 0;

    return StatusCode::SUCCESS;
}

StatusCode ReadTriggerDecision::execute(const EventContext& /* ctx */) const {

    // Here we check whether the trigger named by the user fired for this event
    // and increment the counter if it did. 
    if (m_trigDec->isPassed(m_triggerName)) ++m_triggerCounter;

    // One can get a list of all of the triggers available in the event
    // by doing:
    // const Trig::ChainGroup* chain = m_trigDec->getChainGroup("HLT.*");
    // const std::vector<std::string> fired_triggers = chain->getListOfTriggers();
    // See https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigDecisionTool#Athena
    // for more details

    return StatusCode::SUCCESS;

}

// Print the contents of the map
StatusCode ReadTriggerDecision::finalize() {
    ATH_MSG_INFO("==========================");
    ATH_MSG_INFO("SUMMARY OF TRIGGER COUNTS:");
    ATH_MSG_INFO("==========================");
    ATH_MSG_INFO("Trigger with name " << m_triggerName << " fired for " << std::to_string(m_triggerCounter) << " events");

    return StatusCode::SUCCESS;
}
