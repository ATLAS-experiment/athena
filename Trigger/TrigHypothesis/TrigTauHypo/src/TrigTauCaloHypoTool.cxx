/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:     TrigTauCaloHypoTool.cxx
 * PACKAGE:  Trigger/TrigHypothesis/TrigTauHypo
 *
 * AUTHORS:   P.O. DeViveiros, J.Y. Beaucamp
 * CREATED:   Sometime in early 2015
 *
 * DESCRIPTION: Implementation of CaloMVA cuts. Based on the original
 *              TrigTauGenericHypo.
 *
 *********************************************************************/

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTau/TauJetContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TrigTauCaloHypoTool.h"


using namespace TrigCompositeUtils;

TrigTauCaloHypoTool::TrigTauCaloHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent), m_decisionId(HLT::Identifier::fromToolName(name)) 
{

}


StatusCode TrigTauCaloHypoTool::initialize()
{
    if(!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

    ATH_MSG_DEBUG("Tool configured for chain/id: " << m_decisionId);
    ATH_MSG_DEBUG(" - PtMin: " << m_ptMin.value());
    ATH_MSG_DEBUG(" - AcceptAll: " << m_acceptAll.value());

    return StatusCode::SUCCESS;
}


bool TrigTauCaloHypoTool::decide(const ITrigTauJetHypoTool::ToolInfo& input) const
{
    // Monitoring config
    auto passedCuts = Monitored::Scalar<unsigned int>("CutCounter", 0);
    auto pT = Monitored::Scalar<float>("pT", 0);
    auto mon = Monitored::Group(m_monTool, passedCuts, pT);

    
    // Get TauJet collection
    const xAOD::TauJetContainer* tauJets = input.tauContainer;

    if(tauJets->empty()){
        ATH_MSG_DEBUG("No taus in input collection: Rejecting");
        return false;
    }


    // Tau pass flag
    bool pass = false;

    if(m_acceptAll) {
        pass = true;
        ATH_MSG_DEBUG("AcceptAll property is set: taking all events");
    }

    
    // There should only be a single TauJet in the container (one TauJet in the view);
    // just in case we still run the loop
    for(const xAOD::TauJet* tau : *tauJets) {
        ATH_MSG_DEBUG(" New HLT TauJet candidate");

        ATH_MSG_DEBUG(" pT: " << tau->pt() / Gaudi::Units::GeV);

        // Evaluate minimum pT cut
        if(!(tau->pt() > m_ptMin)) continue;
        passedCuts++;
        pT = tau->pt() / Gaudi::Units::GeV;
        
        pass = true;
    
        ATH_MSG_DEBUG(" Pass hypo tool: " << pass);
    }
    
    return pass;
}


StatusCode TrigTauCaloHypoTool::decide(std::vector<ITrigTauJetHypoTool::ToolInfo>& input) const
{
    for(ITrigTauJetHypoTool::ToolInfo& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
                addDecisionID(m_decisionId, i.decision);
            }
        }
    }
    return StatusCode::SUCCESS;
}

