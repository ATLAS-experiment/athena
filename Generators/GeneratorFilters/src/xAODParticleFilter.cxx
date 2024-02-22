/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Allows the user to search for particles with specified kinematics.
// It will pass if there is an particle with pT and eta or E in the specified
// range
#include "GeneratorFilters/xAODParticleFilter.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

xAODParticleFilter::xAODParticleFilter(const std::string &name,
                                       ISvcLocator *pSvcLocator)
    : GenFilter(name, pSvcLocator) {}

StatusCode xAODParticleFilter::filterInitialize()
{
  ATH_MSG_INFO("Ptcut=" << m_Ptmin);
  ATH_MSG_INFO("Etacut=" << m_EtaRange);
  ATH_MSG_INFO("Energycut=" << m_EnergyRange);
  ATH_MSG_INFO("PDG=" << m_PDGID);
  ATH_MSG_INFO("StatusReq=" << m_StatusReq);
  ATH_MSG_INFO("MinParts=" << m_MinParts);
  ATH_MSG_INFO("Exclusive=" << m_Exclusive);
  return StatusCode::SUCCESS;
}

StatusCode xAODParticleFilter::filterEvent()
{
  int nParts = 0;

// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and 
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }

  // Loop over all particles in the event and build up the grid
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* pitr =  (*xTruthParticleContainer)[iPart];

      if (std::abs(pitr->pdgId()) != m_PDGID ||
          !(m_StatusReq == -1 || pitr->status() == m_StatusReq))
        continue;
      if (pitr->pt() >= m_Ptmin && std::abs(pitr->eta()) <= m_EtaRange &&
          pitr->e() <= m_EnergyRange)
      {
        if ((!m_Exclusive) && (m_MinParts == 1))
          return StatusCode::SUCCESS; // Found at least one particle and we have an inclusive requirement

        // Count only particles not decaying to themselves
        bool notSelfDecay = true;
        if (pitr->decayVtx())
        {
          const xAOD::TruthVertex *decayVertex = pitr->decayVtx();
          int outgoing_particles = decayVertex->nOutgoingParticles();
          for (int part = 0; part < outgoing_particles;
               part++)
          {
            const xAOD::TruthParticle *child =
                decayVertex->outgoingParticle(part);
            if (child->pdgId() != pitr->pdgId())
              continue;
            if (child == pitr)
              continue;
            if (HepMC::is_simulation_particle(child))
              continue;
            notSelfDecay = false;
            break;
          }
        }
        if (notSelfDecay)
          nParts++;
      }
  }
  if (m_Exclusive)
  {
    setFilterPassed(nParts == m_MinParts);
  }
  else
  {
    setFilterPassed(nParts >= m_MinParts);
  }
  return StatusCode::SUCCESS;
}
