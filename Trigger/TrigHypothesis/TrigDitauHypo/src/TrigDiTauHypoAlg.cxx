/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "TrigDiTauHypoAlg.h"


using namespace TrigCompositeUtils;

TrigDiTauHypoAlg::TrigDiTauHypoAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : ::HypoBase(name, pSvcLocator)
{}


StatusCode TrigDiTauHypoAlg::initialize() {
    ATH_CHECK(m_hypoTools.retrieve());
    ATH_CHECK(m_DiTauJets_key.initialize());

    // TauJet are made in views, so they are not in the EvtStore: hide them
    renounce(m_DiTauJets_key);

    return StatusCode::SUCCESS;
}


StatusCode TrigDiTauHypoAlg::execute(const EventContext& context) const
{
    ATH_MSG_DEBUG("Executing " << name());

    // Retrieve previous decisions from the previous step
    SG::ReadHandle<DecisionContainer> previousDecisionsHandle = SG::makeHandle(decisionInput(), context);
    if(!previousDecisionsHandle.isValid()) {
        ATH_MSG_DEBUG("No implicit RH for previous decisions " << decisionInput().key() << ": is this expected?");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Running with " << previousDecisionsHandle->size() << " previous decisions");


    // Create output decision handle
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), context);


    // Prepare inputs for the decision tools
    std::vector<ITrigDiTauHypoTool::ToolInfo> toolInput;
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
        SG::ReadHandle<xAOD::DiTauJetContainer> diTauHandle = ViewHelper::makeHandle(*viewEL, m_DiTauJets_key, context);
        if(!diTauHandle.isValid()) {
            ATH_MSG_WARNING("Something is wrong, missing TauJet container! Continuing anyways skipping view");
            continue;
        }
        ATH_MSG_DEBUG("Tau handle size: " << diTauHandle->size());
        if(diTauHandle->size() != 1) {
            ATH_MSG_DEBUG("Something is wrong, an unexpected number of taus was found (expected 1), continuing anyways skipping view");
            continue;
        }


        // Create new decision
        Decision* newDecision = newDecisionIn(outputHandle.ptr(), hypoAlgNodeName());
        TrigCompositeUtils::linkToPrevious(newDecision, decisionInput().key(), counter);

        ElementLink<xAOD::DiTauJetContainer> newDiTauEL = ViewHelper::makeLink(*viewEL, diTauHandle, 0);
        ATH_CHECK(newDiTauEL.isValid());
        newDecision->setObjectLink(featureString(), newDiTauEL);

        // Create tool input
        toolInput.emplace_back(newDecision, roi, diTauHandle.cptr(), previousDecision);

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
