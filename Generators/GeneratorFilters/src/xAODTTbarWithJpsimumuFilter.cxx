/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/xAODTTbarWithJpsimumuFilter.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"

//---------------------------------------------------------------------------
StatusCode xAODTTbarWithJpsimumuFilter::filterInitialize()
{
   CHECK(m_truthPartContKey.initialize());
   ATH_MSG_INFO("Initialized");
   return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode xAODTTbarWithJpsimumuFilter::filterFinalize()
{
    ATH_MSG_INFO(" Events out of " << m_nPass + m_nFail << " passed the filter");
    return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode xAODTTbarWithJpsimumuFilter::filterEvent()
{
    //---------------------------------------------------------------------------

    bool pass = false;
    bool isjpsi = false;
// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  // Loop over all truth particles in the container
  for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {
            if (std::abs(pitr->pdgId())!=MC::JPSI) continue;
            if (HepMC::is_simulation_particle(pitr)) continue;
            if(!isLeptonDecay(pitr, MC::MUON)) continue;
            if (!passJpsiSelection(pitr)) continue;
            isjpsi = true;

        } /// loop on particles


    if (m_selectJpsi && isjpsi)
        pass = true;

    setFilterPassed(pass);
    return StatusCode::SUCCESS;
}

// ========================================================
bool xAODTTbarWithJpsimumuFilter::isLeptonDecay(const xAOD::TruthParticle *part, int type) const
{
    auto end = part->decayVtx();
    if (!end)
        return true;
    for (size_t thisChild_id = 0; thisChild_id < end->nOutgoingParticles(); thisChild_id++)
    {
        auto p = end->outgoingParticle(thisChild_id);
        if (std::abs(p->pdgId()) != type)
            return false;
    }
    return true;
}

// ========================================================
bool xAODTTbarWithJpsimumuFilter::passJpsiSelection(const xAOD::TruthParticle *part) const
{
    double pt = part->pt();
    double eta = std::abs(part->eta());

    if (pt < m_JpsiPtMinCut)
        return false;
    if (eta > m_JpsiEtaMaxCut)
        return false;

    return true;
}
