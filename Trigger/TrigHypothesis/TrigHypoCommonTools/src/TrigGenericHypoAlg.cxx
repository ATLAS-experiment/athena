/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "TrigGenericHypoAlg.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

TrigGenericHypoAlg::TrigGenericHypoAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  ::HypoBase(name, pSvcLocator) {}

StatusCode TrigGenericHypoAlg::initialize() {
  ATH_CHECK( m_hypoTools.retrieve() );
  ATH_CHECK(m_trigCompKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TrigGenericHypoAlg::execute(const EventContext& context) const {

  // Retrieve previous decision
    auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
    ATH_CHECK( previousDecisionsHandle.isValid() );
    ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");

    SG::WriteHandle<TrigCompositeUtils::DecisionContainer> outputHandle 
        = TrigCompositeUtils::createAndStore(decisionOutput(), context ); 
    TrigCompositeUtils::DecisionContainer* decisions = outputHandle.ptr();

    std::vector<TrigGenericHypoTool::HypoToolInfo> hypoToolInput;
    for (const TrigCompositeUtils::Decision* previousDecision: *previousDecisionsHandle) {
      TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
      TrigCompositeUtils::decisionIDs(previousDecision, previousDecisionIDs);
      
      SG::ReadHandle<xAOD::TrigCompositeContainer> trigComposite(m_trigCompKey, context);
      if(trigComposite->size()==0) continue;
      if(trigComposite->size() > 1){
	ATH_MSG_ERROR("Expect at most 1 TrigComposite, but received: "<<trigComposite->size());//should have at most 1 trig composite
	return StatusCode::FAILURE;
      }
      const xAOD::TrigComposite* tc = trigComposite->at(0);
      ElementLink<TrigCompositeUtils::DecisionContainer> featureLink = TrigCompositeUtils::decisionToElementLink(tc, context);
      
      auto newd = TrigCompositeUtils::newDecisionIn( decisions, TrigCompositeUtils::hypoAlgNodeName() );
      hypoToolInput.emplace_back(newd, tc, previousDecision );
      newd -> setObjectLink( TrigCompositeUtils::featureString(), featureLink );
      TrigCompositeUtils::linkToPrevious( newd, previousDecision, context );
    }

    for ( auto & tool: m_hypoTools ) {
        ATH_CHECK( tool->decide( hypoToolInput ) );    
    } 
  
    ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
    return StatusCode::SUCCESS;
}
