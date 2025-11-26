/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthViews/ViewHelper.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "TrigTauTrackingHypoAlg.h"


using namespace TrigCompositeUtils;

TrigTauTrackingHypoAlg::TrigTauTrackingHypoAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : ::HypoBase(name, pSvcLocator)
{

}


StatusCode TrigTauTrackingHypoAlg::initialize()
{
    ATH_CHECK(m_hypoTools.retrieve());
    ATH_CHECK(m_tracksKey.initialize());
    ATH_CHECK(m_roiKey.initialize(SG::AllowEmpty));
    
    // Track particles and RoIs are made in views, so they are not in the EvtStore: hide them
    renounce(m_tracksKey);
    renounce(m_roiKey);

    return StatusCode::SUCCESS;
}


StatusCode TrigTauTrackingHypoAlg::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Executing " << name());
    
    // Retrieve previous decisions (from the previous step)
    SG::ReadHandle<DecisionContainer> previousDecisionsHandle(decisionInput(), ctx);
    if(!previousDecisionsHandle.isValid()) {
        ATH_MSG_DEBUG("No implicit RH for previous decisions " << decisionInput().key() << ": is this expected?");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Running with " << previousDecisionsHandle->size() << " previous decisions");


    // Create output decision handle
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), ctx);


    // Prepare inputs for the decision tools
    std::vector<ITrigTauTrackingHypoTool::ToolInfo> toolInput;
    int counter = -1;
    for(const xAOD::TrigComposite* previousDecision : *previousDecisionsHandle) {
        counter++;

        // Get View
        const ElementLink<ViewContainer> viewEL = previousDecision->objectLink<ViewContainer>(viewString());
        ATH_CHECK(viewEL.isValid());

        // Get RoI
        const TrigRoiDescriptor *roi = nullptr;
        if(!m_roiKey.empty()) {
            SG::ReadHandle<TrigRoiDescriptorCollection> roiHandle = ViewHelper::makeHandle(*viewEL, m_roiKey, ctx);
            ATH_CHECK(roiHandle.isValid());
            if(roiHandle->size() != 1) {
                ATH_MSG_ERROR("Expected exactly one updated ROI");
                return StatusCode::FAILURE;
            }
            roi = roiHandle->at(0);
        } else {
           LinkInfo<TrigRoiDescriptorCollection> roiEL = findLink<TrigRoiDescriptorCollection>(previousDecision, initialRoIString());
           ATH_CHECK(roiEL.isValid());
           roi = *roiEL.link;
        }

        // Get track particles
        SG::ReadHandle<xAOD::TrackParticleContainer> tracksHandle = ViewHelper::makeHandle(*viewEL, m_tracksKey, ctx);
        ATH_CHECK(tracksHandle.isValid());
        ATH_MSG_DEBUG("Tracks handle size: " << tracksHandle->size());


        // Create new decision
        Decision* newDecision = newDecisionIn(outputHandle.ptr(), hypoAlgNodeName());
        TrigCompositeUtils::linkToPrevious(newDecision, decisionInput().key(), counter);
        if(tracksHandle->size()) {
            // If we have tracks in the RoI, link them as the new decision's feature
            ElementLink<xAOD::TrackParticleContainer> newTracksEL = ViewHelper::makeLink(*viewEL, tracksHandle, 0);
            ATH_CHECK(newTracksEL.isValid());
            newDecision->setObjectLink(featureString(), newTracksEL);
        } else {
            // If not, use the new decision as the feature
            ElementLink<DecisionContainer> decisionEL = decisionToElementLink(newDecision, ctx);
            ATH_CHECK(decisionEL.isValid());
            newDecision->setObjectLink(featureString(), decisionEL);
        }

        // Create tool input
        toolInput.emplace_back(newDecision, roi, tracksHandle.cptr(), previousDecision);

        ATH_MSG_DEBUG("Added view/roi/tracks/previous decision to new decision " << counter << " for view " << (*viewEL)->name());
    }

    ATH_MSG_DEBUG("Found " << toolInput.size() << " inputs to tools");


    // Execute decisions from all tools
    for(auto& tool : m_hypoTools) {
        ATH_CHECK(tool->decide(toolInput));
    }


    ATH_CHECK(hypoBaseOutputProcessing(outputHandle));

    return StatusCode::SUCCESS;
}
