/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODSameParticleHardScatteringFilter.h"

xAODSameParticleHardScatteringFilter::xAODSameParticleHardScatteringFilter(const std::string &name, ISvcLocator *pSvcLocator)
    : GenFilter(name, pSvcLocator)
{
    declareProperty("PDGParent", m_PDGParent);
    declareProperty("PDGChild", m_PDGChild);
}

StatusCode xAODSameParticleHardScatteringFilter::filterInitialize()
{
    if (m_PDGParent.size() == 0)
        ATH_MSG_ERROR("PDGParent[] not set ");
    if (m_PDGChild.size() == 0)
        ATH_MSG_ERROR("PDGChild[] not set ");
    for (int i = 0; i < int(m_PDGParent.size()); i++)
        ATH_MSG_DEBUG("PDGParent[" << i << "] = " << m_PDGParent[i]);
    for (int i = 0; i < int(m_PDGChild.size()); i++)
        ATH_MSG_DEBUG("PDGChild[" << i << "] = " << m_PDGChild[i]);
    return StatusCode::SUCCESS;
}

StatusCode xAODSameParticleHardScatteringFilter::filterEvent()
{
    ATH_MSG_DEBUG(" SameParticleHardScattering filtering for: Parent --> " << m_PDGParent[0]
                                                                           << " and parent " << -m_PDGParent[0]
                                                                           << ", Child --> " << m_PDGChild[0]);
    int N_Parent[2];
    N_Parent[0] = 0;
    N_Parent[1] = 0;

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
            const xAOD::TruthParticle* part =  (*xTruthParticleContainer)[iPart];

            int id = part->pdgId();
            if (std::abs(id) != m_PDGChild[0])
                continue; // searching for only b-quarks

            // a pointer to the production vertex
            const xAOD::TruthVertex* productionVtx = part->prodVtx();

            // Verify if we got a valid pointer and retrieve the number of parents
            if (!productionVtx)
                continue;
                // Incoming particle range check

            if (productionVtx->nIncomingParticles() < 2) //  we are looking for excited tau-leptons produced in b-quark b-antiquark scattering
                continue;
            for(size_t thisParent_id=0; thisParent_id < part->prodVtx()->nIncomingParticles(); thisParent_id++)
            {
                auto thisParent = part->prodVtx()->incomingParticle(thisParent_id);
                ATH_MSG_DEBUG(" SelectBQuarkScattering Filter: parent ==> " << thisParent->pdgId() << " child ===> " << part->pdgId());
                if (thisParent->pdgId() == m_PDGParent[0])
                {
                    N_Parent[0]++;
                }
                if (thisParent->pdgId() == -m_PDGParent[0])
                {
                    N_Parent[1]++;
                }
            }
        } //loop over TruthParticles
    
    setFilterPassed(N_Parent[0] >= 1 && N_Parent[1] >= 1);
    return StatusCode::SUCCESS;
}
