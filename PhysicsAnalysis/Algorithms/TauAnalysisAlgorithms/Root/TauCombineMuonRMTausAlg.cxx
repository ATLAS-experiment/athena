/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Qichen Dong


#include <TauAnalysisAlgorithms/TauCombineMuonRMTausAlg.h>

// unnamed namespace for helpers
namespace {
    const static SG::ConstAuxElement::Decorator<char> decSelection("SelectedByMuonRemovalCombination");
}

namespace CP
{

    StatusCode TauCombineMuonRMTausAlg::initialize ()
    {
        ATH_MSG_INFO("Initializing " << name() << "...");
        ANA_CHECK (m_tauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_MuonRMtauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_outputTauHandle.initialize (m_systematicsList));
        ANA_CHECK (m_systematicsList.initialize());
        return StatusCode::SUCCESS;
    }


    StatusCode TauCombineMuonRMTausAlg::execute ()
    {
        for (const auto& sys : m_systematicsList.systematicsVector())
        {
            const xAOD::TauJetContainer *taus = nullptr;
            const xAOD::TauJetContainer *muonrm_taus = nullptr;
            ANA_CHECK (m_tauHandle.retrieve (taus, sys));
            ANA_CHECK (m_MuonRMtauHandle.retrieve (muonrm_taus, sys));
            for (const xAOD::TauJet* tau : *taus)         decSelection(*tau) = false;
            for (const xAOD::TauJet* tau : *muonrm_taus)  decSelection(*tau) = false;
            std::vector<const xAOD::TauJet*> combined_taus_vec = TauAnalysisTools::combineTauJetsWithMuonRM (taus, muonrm_taus);
            // !shallow copy seems to break the subsequent algorithms. Deep copy is needed.
            xAOD::TauJetContainer* outputTauCont = new xAOD::TauJetContainer();
            xAOD::TauJetAuxContainer* outputTauContAux = new xAOD::TauJetAuxContainer();
            outputTauCont->setStore(outputTauContAux);
            for (const xAOD::TauJet* tau : combined_taus_vec){
                decSelection(*tau) = true;
                xAOD::TauJet* newTau = new xAOD::TauJet();
                *newTau = *tau;
                outputTauCont->push_back(newTau);
            }
            ANA_CHECK (evtStore()->record (outputTauCont,    m_outputTauHandle.getName (sys)));
            ANA_CHECK (evtStore()->record (outputTauContAux, m_outputTauHandle.getName (sys) + "Aux."));

            // auto viewCopy = std::make_unique<ConstDataVector<xAOD::TauJetContainer>> (SG::VIEW_ELEMENTS);
            // for (const xAOD::TauJet* tau : combined_taus_vec){
            //     decSelection(*tau) = true;
            //     viewCopy->push_back (tau);
            // }
            // ANA_CHECK (evtStore()->record (viewCopy.release(), m_outputTauHandle.getName (sys)));
        }
        return StatusCode::SUCCESS;
    }
}
