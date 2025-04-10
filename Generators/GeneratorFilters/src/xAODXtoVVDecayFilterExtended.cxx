/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODXtoVVDecayFilterExtended.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODXtoVVDecayFilterExtended::filterInitialize()
{
    CHECK(m_truthEventKey.initialize());
    ATH_MSG_INFO("PDGGrandParent(H) = " << m_PDGGrandParent << " will scan all ancestors to find PDGGrandParent");
    ATH_MSG_INFO("PDGParent(V)      = " << m_PDGParent );
    if (m_PDGChild1.empty())
        ATH_MSG_ERROR("PDGChild1[] not set ");
    if (m_PDGChild2.empty())
        ATH_MSG_ERROR("PDGChild2[] not set ");
    for (size_t i = 0; i < m_PDGChild1.size(); ++i)
        ATH_MSG_INFO("PDGChild1[" << i << "] = " << m_PDGChild1[i]);
    for (size_t i = 0; i < m_PDGChild2.size(); ++i)
        ATH_MSG_INFO("PDGChild2[" << i << "] = " << m_PDGChild2[i]);
    ATH_MSG_INFO("UseStatusParent  " << m_UseStatusParent);
    ATH_MSG_INFO("StatusParent  " << m_StatusParent);

    // init
    m_nHtoVV = 0;
    m_nGoodHtoVV = 0;
    return StatusCode::SUCCESS;
}

StatusCode xAODXtoVVDecayFilterExtended::filterFinalize()
{
    ATH_MSG_INFO("Statistics of X->VV, V->decay scanning all ancestors of V in order to find X");
    ATH_MSG_INFO("  ALL  X->VV  " << m_nHtoVV);
    ATH_MSG_INFO("  Good X->VV  " << m_nGoodHtoVV);
    if (m_nHtoVV != 0)
        ATH_MSG_INFO("  Fraction   " << double(m_nGoodHtoVV) / double(m_nHtoVV));
    return StatusCode::SUCCESS;
}

StatusCode xAODXtoVVDecayFilterExtended::filterEvent()
{
    bool okPDGChild1 = false;
    bool okPDGChild2 = false;
    int nGoodParent = 0;

    // Retrieve full TruthEventContainer container
    SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_truthEventKey};
    CHECK( xTruthEventContainer.isValid() );

    for (const xAOD::TruthEvent* event : *xTruthEventContainer)
    {
        unsigned int nPart = event->nTruthParticles();
        for (unsigned int iPart = 0; iPart < nPart; ++iPart)
        {
            const xAOD::TruthParticle *pitr = event->truthParticle(iPart);
            if (std::abs(pitr->pdgId()) == m_PDGParent)
            {
                if((!m_UseStatusParent && !MC::isDecayed(pitr)) || (m_UseStatusParent && pitr->status() != m_StatusParent)) continue;
                bool isGrandParentOK = RunHistory(pitr);
                ATH_MSG_DEBUG(" Grand Parent is OK? " << isGrandParentOK);
                if (!isGrandParentOK)
                    continue;
                ++nGoodParent;
                FindAncestor(pitr->decayVtx(), m_PDGParent, okPDGChild1, okPDGChild2);
            }
        }
    }
    ATH_MSG_DEBUG("Result " << nGoodParent << " " << okPDGChild1 << " " << okPDGChild2);

    if (nGoodParent == 2)
    {
        ++m_nHtoVV;
        if (okPDGChild1 && okPDGChild2)
        {
            ++m_nGoodHtoVV;
            return StatusCode::SUCCESS;
        }
    }

    // If we get here we have failed
    setFilterPassed(false);
    return StatusCode::SUCCESS;
}

// Runs the history of ancestors and returns TRUE if it finds the
// m_PDGGrandParent in the list of ansestors
bool xAODXtoVVDecayFilterExtended::RunHistory(const xAOD::TruthParticle *pitr)
{
    if (!pitr->prodVtx())
    {
        ATH_MSG_DEBUG("No History for this case");
        return false;
    }
    if (pitr->prodVtx()->nIncomingParticles() == 0)
    {
        ATH_MSG_DEBUG("No mother for this case");
        return false;
    }
    int result = 999;

    // Check if the first mother is ok
    pitr = CheckGrandparent(pitr, result);
    ATH_MSG_DEBUG("Pointer PDG ID: " << pitr->pdgId());
    if (std::abs(pitr->pdgId()) != m_PDGGrandParent && std::abs(pitr->pdgId()) != m_PDGParent)
        return false;
    if (result == m_PDGGrandParent)
        return true;

    //set pitr_current to pitr of grand parent 
    auto pitr_current = pitr;
    while (result >= 0)
    {
        pitr_current = CheckGrandparent(pitr_current, result);
        ATH_MSG_DEBUG("Pointer PDG ID: " << pitr->pdgId());
        if (std::abs(pitr_current->pdgId()) != m_PDGGrandParent && std::abs(pitr_current->pdgId()) != m_PDGParent)
            return false;

        if (result == m_PDGGrandParent)
            return true;
    }

    return false;
}

// checks whether the grandparent of a given particle is m_PDGGrandParent
// it returns the first mother
const xAOD::TruthParticle *xAODXtoVVDecayFilterExtended::CheckGrandparent(const xAOD::TruthParticle *pitr, int &result)
{

    if (!pitr->prodVtx())
    {
        ATH_MSG_DEBUG("No ancestor for this case");
        result = -1;
        return NULL;
    }
    bool isGrandParentOK = false;

    if (pitr->prodVtx()->nIncomingParticles() == 0)
    {
        ATH_MSG_DEBUG("No mother for this case");
        result = -2;
        return NULL;
    }

    int n_mothers = 1;

    for (size_t thisMother_id = 0; thisMother_id < pitr->prodVtx()->nIncomingParticles(); thisMother_id++)
    {
        auto thisMother = pitr->prodVtx()->incomingParticle(thisMother_id);
        ATH_MSG_DEBUG("Now on this mother: " << (thisMother)->pdgId() << " " << n_mothers);
        if ((thisMother)->pdgId() != m_PDGGrandParent && std::abs((thisMother)->pdgId()) != m_PDGParent)
            break;
        if ((thisMother)->pdgId() == m_PDGGrandParent && n_mothers == 1)
        {
            isGrandParentOK = true;
        }
        n_mothers++;
    }

    if (isGrandParentOK)
    {
        result = m_PDGGrandParent;
    }
    else
    {
        result = 0;
    }

    return pitr->prodVtx()->incomingParticle(0);
}

void xAODXtoVVDecayFilterExtended::FindAncestor(const xAOD::TruthVertex *searchvertex,
                                                int targetPDGID, bool &okPDGChild1, bool &okPDGChild2)
{
    if (!searchvertex)
        return;
    for (size_t thisMother_id = 0; thisMother_id < searchvertex->nOutgoingParticles(); thisMother_id++)
    {
        auto thisAncestor = searchvertex->outgoingParticle(thisMother_id);

        if (std::abs(thisAncestor->pdgId()) == targetPDGID)
        { // same particle as parent
            FindAncestor(thisAncestor->decayVtx(), targetPDGID, okPDGChild1, okPDGChild2);
        }
        else
        {
            if (!okPDGChild1)
            {
                for (size_t i = 0; i < m_PDGChild1.size(); ++i)
                {
                    if (std::abs(thisAncestor->pdgId()) == m_PDGChild1[i])
                    {
                        okPDGChild1 = true;
                        break;
                    }
                }
                if (okPDGChild1)
                    break;
            }
            if (!okPDGChild2)
            {
                for (size_t i = 0; i < m_PDGChild2.size(); ++i)
                {
                    if (std::abs(thisAncestor->pdgId()) == m_PDGChild2[i])
                    {
                        okPDGChild2 = true;
                        break;
                    }
                }
                if (okPDGChild2)
                    break;
            }
        }
    }
}
