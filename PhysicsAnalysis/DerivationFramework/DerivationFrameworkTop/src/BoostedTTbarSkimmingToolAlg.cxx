/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedTTbarSkimmingToolAlg.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TLorentzVector.h"


DerivationFramework::BoostedTTbarSkimmingToolAlg::
BoostedTTbarSkimmingToolAlg(const std::string& t,
                            const std::string& n,
                            const IInterface* p)
  : base_class(t, n, p)
{
}

StatusCode DerivationFramework::BoostedTTbarSkimmingToolAlg::finalize()
{
    ATH_MSG_INFO("Processed "<< m_ntot <<" events, "<< m_npass<<" events passed filter ");
    return StatusCode::SUCCESS;
}

bool DerivationFramework::BoostedTTbarSkimmingToolAlg::eventPassesFilter() const
{
    ++m_ntot;


    const xAOD::TruthParticleContainer* truth = nullptr;
    if (evtStore()->retrieve(truth, "TruthParticles").isFailure()) {
        ATH_MSG_ERROR("Failed to retrieve TruthParticles");
        return false;
    }

    TLorentzVector top, antitop;

    for (const auto* p : *truth) {
        if (std::abs(p->pdgId()) != 6) continue;

        bool hasW = false;
        bool hasB = false;
        for (unsigned int ic = 0; ic < p->nChildren(); ++ic) {
            const xAOD::TruthParticle* child = p->child(ic);
            if (!child) continue;
            if (std::abs(child->pdgId()) == 24) hasW = true;
            if (std::abs(child->pdgId()) == 5)  hasB = true;
        }
        if (!hasW || !hasB) continue;

        TLorentzVector vec;
        vec.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), p->m());
        if (p->pdgId() > 0) top = vec;
        else antitop = vec;
    }

    double mass_ttbar = -1.;
    if (top.Pt() > 0 && antitop.Pt() > 0) mass_ttbar = (top + antitop).M();

    if (mass_ttbar <= m_ttbarCut) return false;

    ++m_npass;
    return true;
}