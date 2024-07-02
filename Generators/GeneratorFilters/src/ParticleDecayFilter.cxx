/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/ParticleDecayFilter.h"

ParticleDecayFilter::ParticleDecayFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{ 

}

StatusCode ParticleDecayFilter::filterInitialize()
{
    ATH_MSG_INFO("ParticleDecayFilter::filterInitialize()");

    // initialize the ReadHandleKey
    ATH_CHECK( m_truthEventsKey.initialize() );

    return StatusCode::SUCCESS;
}

StatusCode ParticleDecayFilter::filterFinalize()
{
    ATH_MSG_INFO("ParticleDecayFilter::filterFinalize()");
    return StatusCode::SUCCESS;
}


StatusCode ParticleDecayFilter::filterEvent(){

    ATH_MSG_DEBUG("ParticleDecayFilter::filterEvent()");

    //Create child targets - a map of pdgId along with how many particles
    //with that pdgId that we want
    std::map<int, unsigned int> childTargets;

    //loop over children pdgId and setup counters for each particle type
    for (auto childPdgId : m_pdgIdChildren.value()){
        //for charged parents we don't check the charge of children, in order to support both possible conjugate decay modes
        if (!m_checkCharge) childPdgId = std::abs(childPdgId);
        if (childTargets.end() != childTargets.find(childPdgId)) childTargets[childPdgId] +=1;
        else childTargets[childPdgId] = 1;
    }

    //loop over truth events
    McEventCollection::const_iterator truthEvent;
    for (truthEvent = events()->begin(); truthEvent != events()->end(); ++truthEvent){

        // get the truth particles
        const HepMC::GenEvent* genEvent = (*truthEvent);

        // loop over all particles
        for (auto particle : *genEvent){
            ATH_MSG_DEBUG("particle in the event is " << particle);

            //maps with key pdgId and value of number of particles with that pdgId
            std::map<int, unsigned int> childCounters;
            //counter for any children not in the specified list of children
            int nonListValue = -999;
            childCounters[nonListValue]= 0;

            //loop over children pdgId and setup counters for each particle type
            for (auto childPdgId : m_pdgIdChildren.value()) childCounters[childPdgId] = 0;
                        
            // check if the parent particle id is found
            if (std::abs(particle->pdg_id()) == static_cast<int>(m_pdgIdParent)){
                const HepMC::ConstGenVertexPtr& decayVtx = particle->end_vertex();
                ATH_MSG_DEBUG("Number of children is " << decayVtx->particles_out_size());
                //loop over children and count number of particles with each pdgId
                for(auto thisChild: *decayVtx) {
                    int childPdgId = thisChild->pdg_id();
                    if (!m_checkCharge) childPdgId = std::abs(childPdgId);
                    ATH_MSG_DEBUG("Child " << thisChild);
                    if (childTargets.end() != childTargets.find(childPdgId)) childCounters[childPdgId] += 1;
                    else childCounters[nonListValue] += 1;
                }                

                //check if the targeted number of children of each type have been found
                if (std::all_of(childTargets.begin(), childTargets.end(), [&](const std::pair<unsigned int, unsigned int>& p) { return p.second == childCounters[p.first]; }) && childCounters[nonListValue] == 0){
                    ATH_MSG_DEBUG("Filter passed");
                    setFilterPassed(true);
                    return StatusCode::SUCCESS;
                }
            }// end if particle has parent pdg id  
        }// end loop over particles
    }//loop over truth events

    // if we get here, no particle was found with the required set of children
    setFilterPassed(false);
    return StatusCode::SUCCESS;
}
