/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Qichen Dong


#include <TauAnalysisAlgorithms/TauCombineMuonRMTausAlg.h>

#include "xAODBase/IParticleHelpers.h"

// unnamed namespace for helpers
namespace {
    const SG::ConstAccessor<ElementLink<xAOD::TauJetContainer>> linkAcc("originalTauJet");
}

namespace CP
{

    StatusCode TauCombineMuonRMTausAlg::initialize ()
    {
        ANA_CHECK (m_tauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_MuonRMtauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_outputTauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_tauSelectionDecor.initialize (m_systematicsList, m_tauHandle));
        ANA_CHECK (m_MuonRMtauSelectionDecor.initialize (m_systematicsList, m_MuonRMtauHandle));
        ANA_CHECK (m_systematicsList.initialize());
        return StatusCode::SUCCESS;
    }


    StatusCode TauCombineMuonRMTausAlg::execute (const EventContext& ctx)
    {
        for (const auto& sys : m_systematicsList.systematicsVector())
        {
            const xAOD::TauJetContainer *taus = nullptr;
            const xAOD::TauJetContainer *muonrm_taus = nullptr;
            ANA_CHECK (m_tauHandle.retrieve (taus, sys, ctx));
            ANA_CHECK (m_MuonRMtauHandle.retrieve (muonrm_taus, sys, ctx));
            for (const xAOD::TauJet* tau : *taus)         m_tauSelectionDecor.set (*tau, false, sys);
            for (const xAOD::TauJet* tau : *muonrm_taus)  m_MuonRMtauSelectionDecor.set (*tau, false, sys);
            std::vector<const xAOD::TauJet*> combined_taus_vec = TauAnalysisTools::combineTauJetsWithMuonRM (taus, muonrm_taus);
            // !shallow copy seems to break the subsequent algorithms. Deep copy is needed.
            auto outputTauCont = std::make_unique<xAOD::TauJetContainer>();
            auto outputTauContAux = std::make_unique<xAOD::TauJetAuxContainer>();
            outputTauCont->setStore(outputTauContAux.get());
            for (const xAOD::TauJet* tau : combined_taus_vec){
              if(linkAcc.isAvailable(*tau) && !linkAcc(*tau).isValid()){
                ATH_MSG_WARNING("Invalid originalTauJet link for tau with pT="<<tau->pt());
                continue;
              }
              // both decoration handles share the same decoration name, so
              // either can be used for taus from either input container
              m_tauSelectionDecor.set (*tau, true, sys);
              auto newTau = std::make_unique<xAOD::TauJet>();
              newTau->makePrivateStore(*tau);
              if(linkAcc.isAvailable(*tau)){
                auto link = linkAcc(*tau);
                setOriginalObjectLink(**link, *newTau);
              }
              else setOriginalObjectLink(*tau, *newTau);
              outputTauCont->push_back(std::move(newTau));
            }
            ANA_CHECK (m_outputTauHandle.record (std::move (outputTauCont), std::move (outputTauContAux), sys, ctx));

        }
        return StatusCode::SUCCESS;
    }
}
