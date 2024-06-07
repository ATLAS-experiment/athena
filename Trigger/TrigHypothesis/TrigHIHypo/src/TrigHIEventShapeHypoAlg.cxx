/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigHIEventShapeHypoAlg.h"

#include "Gaudi/Property.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

TrigHIEventShapeHypoAlg::TrigHIEventShapeHypoAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator) :
  ::HypoBase(name, pSvcLocator) {}


StatusCode TrigHIEventShapeHypoAlg::initialize() {
  ATH_CHECK(m_hypoTools.retrieve());
  ATH_CHECK(m_esKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TrigHIEventShapeHypoAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  // Retrieve the HI event shape container
  auto esHandle = SG::makeHandle(m_esKey, context);
  ATH_CHECK(esHandle.isValid());
  ATH_MSG_DEBUG("Retrieving HI event shape container using key: " << esHandle.key());
  const xAOD::HIEventShapeContainer* eventShapeContainer = esHandle.get();

  // Retrieve the previous Decisions made before running this hypo
  auto prevDecisionsHandle = SG::makeHandle(decisionInput(), context);
  ATH_CHECK(prevDecisionsHandle.isValid());
  ATH_MSG_DEBUG("Running with " << prevDecisionsHandle->size() << " implicit ReadHandles for previous decisions");
  const TrigCompositeUtils::DecisionContainer* prevDecisions = prevDecisionsHandle.get();

  // Make a new Decisions container which will contain the new Decision object created by this hypo
  SG::WriteHandle<TrigCompositeUtils::DecisionContainer> decisionOutputHandle = TrigCompositeUtils::createAndStore(decisionOutput(), context);
  TrigCompositeUtils::DecisionContainer* newDecisions = decisionOutputHandle.ptr();

  // Make trigger decisions and save to "newDecisions"
  ATH_CHECK(decide(eventShapeContainer, newDecisions, prevDecisions, context));

  // Common debug printing
  ATH_CHECK(hypoBaseOutputProcessing(decisionOutputHandle));

  return StatusCode::SUCCESS;
}


StatusCode TrigHIEventShapeHypoAlg::decide(const xAOD::HIEventShapeContainer* eventShapeContainer,
                                       TrigCompositeUtils::DecisionContainer* newDecisions,
                                       const TrigCompositeUtils::DecisionContainer* oldDecisions,
                                       const EventContext& context) const {

  ATH_MSG_DEBUG("Executing decide() of " << name());
  if (oldDecisions->size() != 1) {
    ATH_MSG_ERROR("TrigHIEventShapeHypoAlg requires there to be exactly one previous Decision object, but found " << oldDecisions->size());
    return StatusCode::FAILURE;
  }
  const TrigCompositeUtils::Decision* previousDecision = oldDecisions->at(0);

  if (eventShapeContainer->size()==0) {
    ATH_MSG_ERROR("There are no HIEventShape objects in the container");
    return StatusCode::FAILURE;
  }

  // Create output Decision object, link it to prevDecision and its "feature"
  auto newDecision = TrigCompositeUtils::newDecisionIn(newDecisions, previousDecision, TrigCompositeUtils::hypoAlgNodeName(), context);
  ElementLink<xAOD::HIEventShapeContainer> esElementLink = ElementLink<xAOD::HIEventShapeContainer>(*eventShapeContainer, /*index*/ 0);
  newDecision->setObjectLink<xAOD::HIEventShapeContainer>(TrigCompositeUtils::featureString(), esElementLink);

  // Get set of active chains
  const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs{
    TrigCompositeUtils::decisionIDs(previousDecision).begin(),
    TrigCompositeUtils::decisionIDs(previousDecision).end()
  };

  for (const auto& tool: m_hypoTools) {
    ATH_MSG_DEBUG("About to decide for " << tool->name());

    auto decisionId = tool->getId();
    if (TrigCompositeUtils::passed(decisionId.numeric(), previousDecisionIDs)) {
      ATH_MSG_DEBUG("Passed previous trigger step");

      bool pass;
      ATH_CHECK(tool->decide(eventShapeContainer, pass));

      if (pass) {
        ATH_MSG_DEBUG("Passed " << tool->name());
        TrigCompositeUtils::addDecisionID(decisionId, newDecision);
      }
      else {
        ATH_MSG_DEBUG("Didn't pass " << tool->name());
      }
    }
    else {
      ATH_MSG_DEBUG("Didn't pass previous trigger step");
    }
  }

  return StatusCode::SUCCESS;
}

