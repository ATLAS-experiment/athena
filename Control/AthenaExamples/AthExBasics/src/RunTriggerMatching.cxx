/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RunTriggerMatching.h"
#include "TrigDecisionTool/ChainGroup.h"
#include <vector>

RunTriggerMatching::RunTriggerMatching(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode RunTriggerMatching::initialize() {

    // Initialise the data handles
    ATH_CHECK(m_containerKey.initialize());
    // Initialise the tool handles
    ATH_CHECK(m_trigDec.retrieve());
    ATH_CHECK(m_matchingTool.retrieve());
    // Initialise counter
    m_matchCounter = 0;
    return StatusCode::SUCCESS;
}

StatusCode RunTriggerMatching::execute(const EventContext& ctx) const {

    // Get the particle containers requested
    SG::ReadHandle<xAOD::IParticleContainer> particles{m_containerKey, ctx};
    if( ! particles.isValid() ) {
        ATH_MSG_ERROR ("Couldn't retrieve IParticles with key: " << m_containerKey.key() );
        return StatusCode::FAILURE;
    }

    // Get the full list of triggers that were available for this event matching the user string
    const Trig::ChainGroup* chain = m_trigDec->getChainGroup(m_triggerString);
    const std::vector<std::string> fired_triggers = chain->getListOfTriggers();

    // For triggers that fired, count events where at least one offline object
    // from the provided container matches that trigger
    for (const std::string& fired : fired_triggers) {
        if (m_trigDec->isPassed(fired)) {
            for ( const xAOD::IParticle* part : *particles) {
                if (m_matchingTool->match(*part, fired, 0.1, false)) {
                    m_matchCounter++;
                    break;
                }
            }
        }
    }

    return StatusCode::SUCCESS;

}

// Print the contents of the map
StatusCode RunTriggerMatching::finalize() {
    ATH_MSG_INFO("=========================");
    ATH_MSG_INFO("SUMMARY OF TRIGGER COUNTS");
    ATH_MSG_INFO("=========================");
    ATH_MSG_INFO("Number of events where a trigger matching the user string");
    ATH_MSG_INFO("fired and is matched to at least one offline object in " << m_containerKey.key() << " : " << std::to_string(m_matchCounter));
    
    return StatusCode::SUCCESS;
}





