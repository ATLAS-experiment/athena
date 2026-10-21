/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MlMuonDVtxHypoAlg.h"

#include "StoreGate/ReadDecorHandle.h"

using namespace TrigCompositeUtils; 

StatusCode MlMuonDVtxHypoAlg::initialize()
{
  ATH_CHECK(m_DVOutputKey.initialize());

  ATH_CHECK(m_hypoTools.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode MlMuonDVtxHypoAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG ( "Executing " << name() << "..." );

    // Obtain incoming Decision Objects (one per L1 muon ROI)
    auto previousDecisionsHandle = SG::makeHandle( decisionInput(), ctx );
    ATH_CHECK( previousDecisionsHandle.isValid() );
    ATH_MSG_DEBUG("Running with "<<previousDecisionsHandle->size()<<" previous decisions");

    // New output trigger decisions container
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), ctx );
    auto decisions = outputHandle.ptr();

    if (previousDecisionsHandle->size() == 0) {
        ATH_MSG_DEBUG( "No previous decision, nothing to do.");
        return StatusCode::SUCCESS;
    }
    
    const xAOD::TrigCompositeContainer* dvOutput {nullptr};
    ATH_CHECK(SG::get(dvOutput, m_DVOutputKey, ctx));

    if (dvOutput->size() != 1) {
        ATH_MSG_ERROR("One DV output expected, but " << dvOutput->size() << " found.");
        return StatusCode::FAILURE;
    }

    // Creating a DecisionIDContainer to hold all active chain IDs
    DecisionIDContainer chainIDs;
    
    // Create new output decision object
    Decision* newDecision = newDecisionIn(decisions, hypoAlgNodeName());

    // Link the DV output to the new decision
    newDecision->setObjectLink(featureString(), 
                               ElementLink<xAOD::TrigCompositeContainer>(*dvOutput, 0, ctx));

    // We have only one output decision (event level) and many inputs (L1 muon ROIs),
    // so we link the new decision to all previous decisions (L1 muon ROIs)
    for (const Decision* previousDecision : *previousDecisionsHandle) {
        linkToPrevious(newDecision, previousDecision, ctx);
        decisionIDs(previousDecision, chainIDs);
    }

    MlMuonDVtxHypoTool::Info info{newDecision, 
                                  dvOutput->front()->getDetail<float>("Score"), 
                                  std::move(chainIDs)};

    for (const auto& hypoTool : m_hypoTools) {
        ATH_CHECK(hypoTool->decide(info));
    }

    ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
    return StatusCode::SUCCESS;
}
