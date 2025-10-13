/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrigTauFastTrackHypoTool.h"


using namespace TrigCompositeUtils;

TrigTauFastTrackHypoTool::TrigTauFastTrackHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent), m_decisionId(HLT::Identifier::fromToolName(name))
{

}


StatusCode TrigTauFastTrackHypoTool::initialize()
{
    ATH_MSG_DEBUG("Tool configured for chain/id: " << m_decisionId);

    return StatusCode::SUCCESS;
}


bool TrigTauFastTrackHypoTool::decide(const ITrigTauFastTrackHypoTool::ToolInfo& input) const
{
    // Get RoI descriptor
    ATH_MSG_DEBUG( "Input RoI eta: " << input.roi->eta() << ", phi: " << input.roi->phi() << ", z: " << input.roi->zed());

    // Check the input TrackParticles
    const xAOD::TrackParticleContainer* tracks = input.trackCollection;
    if(!tracks->empty()){
        ATH_MSG_DEBUG("Input Fast TrackParticle collection has size: " << tracks->size());
    }

    // This is (for now) a dummy step, so we won't be applying any decision here
    bool pass = true;
    
    return pass;
}


StatusCode TrigTauFastTrackHypoTool::decide(std::vector<ITrigTauFastTrackHypoTool::ToolInfo>& input) const
{
    for(auto& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
                addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}
