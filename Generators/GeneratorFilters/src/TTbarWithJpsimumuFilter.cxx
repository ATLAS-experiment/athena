/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/TTbarWithJpsimumuFilter.h"

#include "GaudiKernel/MsgStream.h"
#include "TruthUtils/MagicNumbers.h"

//--------------------------------------------------------------------------
TTbarWithJpsimumuFilter::TTbarWithJpsimumuFilter(const std::string& fname,
        ISvcLocator* pSvcLocator)
    : GenFilter(fname,pSvcLocator)

{
    declareProperty("SelectJpsi",m_selectJpsi=true);
    declareProperty("JpsipTMinCut",m_JpsiPtMinCut=0.); /// MeV
    declareProperty("JpsietaMaxCut",m_JpsiEtaMaxCut=5.);
}

//--------------------------------------------------------------------------
TTbarWithJpsimumuFilter::~TTbarWithJpsimumuFilter() {
    /////
}

//---------------------------------------------------------------------------
StatusCode TTbarWithJpsimumuFilter::filterInitialize() {
    ATH_MSG_INFO("Initialized");
    return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode TTbarWithJpsimumuFilter::filterFinalize() {
    ATH_MSG_INFO(" Events out of " << m_nPass+m_nFail << " passed the filter");
    return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode TTbarWithJpsimumuFilter::filterEvent() {
    //---------------------------------------------------------------------------

    bool pass = false;
    bool isjpsi = false;

    for (const HepMC::GenEvent* genEvt : *events_const()) {

        // Loop over all truth particles in the event
        // ===========================================
        for(const auto& part: *genEvt) {
            if (std::abs(part->pdg_id())!=443) continue;
            if (HepMC::is_simulation_particle(part)) continue;
            if(!isLeptonDecay(part,13)) continue;
            if ( !passJpsiSelection(part) ) continue;
            isjpsi=true;
        } /// loop on particles

    } // loop on events (only one at evgen - no PU)

    if (m_selectJpsi && isjpsi) pass = true;
    
    setFilterPassed(pass);
    return StatusCode::SUCCESS;
}

// ========================================================
bool TTbarWithJpsimumuFilter::isLeptonDecay(const HepMC::ConstGenParticlePtr& part, int type) const {
    auto end = part->end_vertex();
    if(!end) return true;
    for (const auto& p: *end) {
        if (std::abs(p->pdg_id()) !=  type ) return false;
    }
    return true;
}

// ========================================================
bool TTbarWithJpsimumuFilter::passJpsiSelection(const HepMC::ConstGenParticlePtr& part) const {

    const HepMC::FourVector& p4 = part->momentum();
    double pt = p4.perp();
    double eta = std::abs(p4.eta());

    if (pt < m_JpsiPtMinCut) return false;
    if (eta > m_JpsiEtaMaxCut) return false;

    return true;

}
