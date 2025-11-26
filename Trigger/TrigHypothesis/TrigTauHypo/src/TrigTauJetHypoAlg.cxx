/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "TrigTauJetHypoAlg.h"


using namespace TrigCompositeUtils;

TrigTauJetHypoAlg::TrigTauJetHypoAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : ::HypoBase(name, pSvcLocator)
{

}


StatusCode TrigTauJetHypoAlg::initialize()
{
    ATH_CHECK(m_hypoTools.retrieve());
    ATH_CHECK(m_tauJetKey.initialize());
    
    // TauJets are made in views, so they are not in the EvtStore: hide them
    renounce(m_tauJetKey); 

    return StatusCode::SUCCESS;
}


StatusCode TrigTauJetHypoAlg::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Executing " << name());

    // Retrieve previous decisions (from the HLT Seeding or previous HLT step)
    SG::ReadHandle<DecisionContainer> previousDecisionsHandle(decisionInput(), ctx);
    if(!previousDecisionsHandle.isValid()) {
        ATH_MSG_DEBUG("No implicit RH for previous decisions " << decisionInput().key() << ": is this expected?");
        return StatusCode::SUCCESS;      
    }
    
    ATH_MSG_DEBUG("Running with " << previousDecisionsHandle->size() << " previous decisions");


    // Create output decision handle
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), ctx);


    // Prepare inputs for the decision tools
    std::vector<ITrigTauJetHypoTool::ToolInfo> toolInput;
    int counter = -1;
    for(const xAOD::TrigComposite* previousDecision : *previousDecisionsHandle) {
        counter++;

        // Get View
        const ElementLink<ViewContainer> viewEL = previousDecision->objectLink<ViewContainer>(viewString());
        ATH_CHECK(viewEL.isValid());

        // Get RoI
        LinkInfo<TrigRoiDescriptorCollection> roiEL = findLink<TrigRoiDescriptorCollection>(previousDecision, roiString());
        ATH_CHECK(roiEL.isValid());
        const TrigRoiDescriptor* roi = *roiEL.link;

        // Get TauJet
        SG::ReadHandle<xAOD::TauJetContainer> tauHandle = ViewHelper::makeHandle(*viewEL, m_tauJetKey, ctx);
        ATH_CHECK(tauHandle.isValid());
        ATH_MSG_DEBUG("Tau handle size: " << tauHandle->size());
        if(tauHandle->size() != 1) {
            ATH_MSG_DEBUG("Something is wrong, an unexpected number of taus was found (expected 1), continuing anyways skipping view");
            continue;
        }


        // Create new decision
        Decision* newDecision = newDecisionIn(outputHandle.ptr(), hypoAlgNodeName());
        TrigCompositeUtils::linkToPrevious(newDecision, decisionInput().key(), counter);
        newDecision->setObjectLink(roiString(), roiEL.link);

        ElementLink<xAOD::TauJetContainer> newTauEL = ViewHelper::makeLink(*viewEL, tauHandle, 0);
        ATH_CHECK(newTauEL.isValid());
        newDecision->setObjectLink(featureString(), newTauEL);

        // Create tool input
        toolInput.emplace_back(newDecision, roi, tauHandle.cptr(), previousDecision);

        ATH_MSG_DEBUG("Added view, roi, tau, previous decision to new decision (" << counter << ") for view " << (*viewEL)->name());
    }

    ATH_MSG_DEBUG("Found " << toolInput.size() << " inputs to tools");

    
    // Execute decisions from all tools
    for(auto& tool : m_hypoTools) {
        ATH_CHECK(tool->decide(toolInput));
    }
 

    ATH_CHECK(hypoBaseOutputProcessing(outputHandle));

    return StatusCode::SUCCESS;
}

