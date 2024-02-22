/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/xAODTTbarWithJpsimumuFilter.h"

#include "GaudiKernel/MsgStream.h"

//--------------------------------------------------------------------------
xAODTTbarWithJpsimumuFilter::xAODTTbarWithJpsimumuFilter(const std::string &fname,
                                                         ISvcLocator *pSvcLocator)
    : GenFilter(fname, pSvcLocator)

{
    declareProperty("SelectJpsi", m_selectJpsi = true);
    declareProperty("JpsipTMinCut", m_JpsiPtMinCut = 0.); /// MeV
    declareProperty("JpsietaMaxCut", m_JpsiEtaMaxCut = 5.);
}

//--------------------------------------------------------------------------
xAODTTbarWithJpsimumuFilter::~xAODTTbarWithJpsimumuFilter()
{
    /////
}

//---------------------------------------------------------------------------
StatusCode xAODTTbarWithJpsimumuFilter::filterInitialize()
{
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
  const xAOD::TruthParticleContainer* xTruthParticleContainer;  
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }
        
  // Loop over all truth particles in the container
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* pitr =  (*xTruthParticleContainer)[iPart];
            if (std::abs(pitr->pdgId())!=443) continue;
            if (HepMC::is_simulation_particle(pitr)) continue;
            if(!isLeptonDecay(pitr, 13)) continue;
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
