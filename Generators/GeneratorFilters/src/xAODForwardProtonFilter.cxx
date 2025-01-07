/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// GeneratorFilters/ForwardProtonFilter
//
// Allows the user to search for forward intact protons originating from
// elastic and diffractive interactions. Event passes filters if there forward
// proton with xi and pt in given range. If double tag option is enables, such
// protons are requested on both sides. single_tagA/single_tagC require protons
// on a given side, regardless what's happenning on the other
//
// Authors:
// Rafal Staszewski   Jul 2011

#include "GeneratorFilters/xAODForwardProtonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

xAODForwardProtonFilter::xAODForwardProtonFilter(const std::string &name, ISvcLocator *pSvcLocator)
    : GenFilter(name, pSvcLocator)
{
}

StatusCode xAODForwardProtonFilter::filterEvent()
{
  bool accepted_A = false;
  bool accepted_C = false;

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;   
  }

  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* pitr =  (*xTruthParticleContainer)[iPart];

      // We're only interested in stable (status == 1) particles

      if (!MC::isStable(pitr))
        continue;
      // We are specifically looking for protons
      const long pid = pitr->pdgId();
      if (pid != MC::PROTON)
        continue;
      const double E = pitr->e();
      const double pz = pitr->pz();
      const double pt = pitr->pt();
      const double xi = (m_E0 - E) / m_E0;

      if (m_xiMin <= xi && xi <= m_xiMax && m_ptMin <= pt && pt <= m_ptMax)
      {
        accepted_C = (pz > 0);
        accepted_A = (pz < 0);
      }
      if (accepted_A)
        ATH_MSG_DEBUG("Found a proton with xi=" << xi << " on side C");
      if (accepted_C)
        ATH_MSG_DEBUG("Found a proton with xi=" << xi << " on side A");
      if (m_DoubleTag && (accepted_A && accepted_C))
        return StatusCode::SUCCESS;

      if (!m_DoubleTag)
       
        // if Single tag is not requested, do or
        if (!m_Single_tagA && !m_Single_tagC && (accepted_A || accepted_C))
          return StatusCode::SUCCESS;

      // if single tag request - check for presence on particular side
      if (m_Single_tagA && accepted_A)
        return StatusCode::SUCCESS;
      if (m_Single_tagC && accepted_C)
        return StatusCode::SUCCESS;
    } // end loop on particles

  // If we get here the event doesn't pass the criteria
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}
