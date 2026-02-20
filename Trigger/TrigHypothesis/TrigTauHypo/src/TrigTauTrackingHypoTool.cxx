/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "TrigTauTrackingHypoTool.h"


using namespace TrigCompositeUtils;

TrigTauTrackingHypoTool::TrigTauTrackingHypoTool(const std::string& type, const std::string& name, const IInterface* parent) 
    : base_class(type, name, parent), m_decisionId(HLT::Identifier::fromToolName(name))
{

}


StatusCode TrigTauTrackingHypoTool::initialize()
{
    ATH_MSG_DEBUG("Tool configured for chain/id: " << m_decisionId);

    return StatusCode::SUCCESS;
}


bool TrigTauTrackingHypoTool::decide(const ITrigTauTrackingHypoTool::ToolInfo& input) const
{
    // Get RoI descriptor
    ATH_MSG_DEBUG("Input RoI eta: " << input.roi->eta() << ", phi: " << input.roi->phi() << ", z: " << input.roi->zed());

    // Check the input track collection
    ATH_MSG_DEBUG("Input Tracks collection has size: " << input.trackParticles->size());

    // This is (for now) a dummy step, so we return an always passing decision
    return true;
}


StatusCode TrigTauTrackingHypoTool::decide(std::vector<ITrigTauTrackingHypoTool::ToolInfo>& input) const
{
    for(ITrigTauTrackingHypoTool::ToolInfo& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
                addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}
