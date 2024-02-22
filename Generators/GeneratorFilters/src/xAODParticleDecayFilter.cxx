/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODParticleDecayFilter.h"

xAODParticleDecayFilter::xAODParticleDecayFilter(const std::string &name, ISvcLocator *pSvcLocator)
    : GenFilter(name, pSvcLocator)
{
}

StatusCode xAODParticleDecayFilter::filterInitialize()
{
    ATH_MSG_INFO("xAODParticleDecayFilter::filterInitialize()");

    // initialize the ReadHandleKey
    ATH_CHECK(m_truthEventsKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode xAODParticleDecayFilter::filterFinalize()
{
    ATH_MSG_INFO("xAODParticleDecayFilter::filterFinalize()");
    return StatusCode::SUCCESS;
}

StatusCode xAODParticleDecayFilter::filterEvent()
{

    ATH_MSG_DEBUG("xAODParticleDecayFilter::filterEvent()");

    //Create child targets - a map of pdgId along with how many particles
    //with that pdgId that we want
    std::map<int, unsigned int> childTargets;

    //loop over children pdgId and setup counters for each particle type
    for (auto childPdgId : m_pdgIdChildren.value())
    {
        //for charged parents we don't check the charge of children, in order to support both possible conjugate decay modes
        if (!m_checkCharge) childPdgId = std::abs(childPdgId);
        if (childTargets.end() != childTargets.find(childPdgId)) childTargets[childPdgId] += 1;
        else childTargets[childPdgId] = 1;
    }

    ATH_MSG_DEBUG("xAODParticleDecayFilter::filterEvent()");
// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }    

  // Loop over all particles in the event 
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* particle =  (*xTruthParticleContainer)[iPart];

            ATH_MSG_DEBUG("pdg code of this particle in the event is " << particle->pdgId() << " with status " << particle->status());
            //maps with key pdgId and value of number of particles with that pdgId
            std::map<int, unsigned int> childCounters;
            //counter for any children not in the specified list of children
            int nonListValue = -999;
            childCounters[nonListValue] = 0;

            //loop over children pdgId and setup counters for each particle type
            for (auto childPdgId : m_pdgIdChildren.value()) childCounters[childPdgId] = 0;

            // check if the parent particle id is found
            if (std::abs(particle->pdgId()) == static_cast<int>(m_pdgIdParent))
            {
                const xAOD::TruthVertex *decayVtx = particle->decayVtx();
                ATH_MSG_DEBUG("Number of children is " << decayVtx->nOutgoingParticles());
                //loop over children and count number of particles with each pdgId
                int outgoing_particles = decayVtx->nOutgoingParticles();
                for (int part = 0; part < outgoing_particles; part++)
                {
                    const xAOD::TruthParticle *thisChild = decayVtx->outgoingParticle(part);
                    int childPdgId = thisChild->pdg_id();
                    if (!m_checkCharge) childPdgId = std::abs(childPdgId);
                    ATH_MSG_DEBUG("Child with id and status " << childPdgId << " " << thisChild->status());
                    if (childTargets.end() != childTargets.find(childPdgId))  childCounters[childPdgId] += 1;
                    else childCounters[nonListValue] += 1;
                }

                //check if the targeted number of children of each type have been found
                if (std::all_of(childTargets.begin(), childTargets.end(), [&](const std::pair<unsigned int, unsigned int> &p) { return p.second == childCounters[p.first]; }) && childCounters[nonListValue] == 0)
                {
                    ATH_MSG_DEBUG("Filter passed");
                    setFilterPassed(true);
                    return StatusCode::SUCCESS;
                }
            } // end if particle has parent pdg id
        }     // end loop over particles
    

    // if we get here, no particle was found with the required set of children
    setFilterPassed(false);
    return StatusCode::SUCCESS;
}
