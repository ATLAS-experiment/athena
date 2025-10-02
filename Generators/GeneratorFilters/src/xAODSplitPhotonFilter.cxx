/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/xAODSplitPhotonFilter.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODSplitPhotonFilter::filterInitialize()
{
    CHECK(m_truthPartContKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode xAODSplitPhotonFilter::filterEvent()
{
  int NPhotons = 0;
  bool GoodFlav = m_dauPdg.size() == 0 ? true : false;

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  // Check for a photon with desired kinematics
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    if ((MC::isPhoton(part)))
    {

      if (part->pt() >= m_Ptmin && part->abseta() <= m_EtaRange)
      {

        // First find a direct photon (not from hadron decay)
        bool fromHadron(false);

        for (size_t thisParent_id = 0; thisParent_id < part->prodVtx()->nIncomingParticles(); thisParent_id++)
        {
          auto parent = part->prodVtx()->incomingParticle(thisParent_id);
          int pdgindex = parent->pdgId();
          if (pdgindex > 100)
          {
            fromHadron = true;
            break;
          }
        }
        if (fromHadron)
          continue;
        const xAOD::TruthVertex *decayVtx = part->decayVtx();
        if (decayVtx && decayVtx->nOutgoingParticles() > 1)
        {

          ATH_MSG_DEBUG("A split photon");

          // find daughters
          for (size_t thisChild_id = 0; thisChild_id < part->decayVtx()->nOutgoingParticles(); thisChild_id++)
          {
            auto child = part->decayVtx()->outgoingParticle(thisChild_id);
            int pdgid = child->pdgId();
            if (std::find(m_dauPdg.begin(), m_dauPdg.end(), abs(pdgid)) != m_dauPdg.end())
              //Argh I should break here... anyway should not waste too much time to continue the loop even when a good daughter is found
              GoodFlav = true;
            ATH_MSG_DEBUG("Daughter : pdg = " << pdgid);
          }
          NPhotons++;
        }
      } // kinematic requirements
    }   // photon
  }     // part

  if (NPhotons >= m_NPhotons && GoodFlav)
    return StatusCode::SUCCESS;
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}
