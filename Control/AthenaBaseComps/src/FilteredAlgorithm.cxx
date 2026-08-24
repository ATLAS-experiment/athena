/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/FilteredAlgorithm.h"

#include <string>
#include <vector>


StatusCode 
FilteredAlgorithm::initialize() 
{
  // Decode the accept, required and veto Algorithms.
  // The logic is the following:
  //  a. The event is accepted if all lists are empty.
  //  b. The event is provisionally accepted if any Algorithm in the 
  //     accept list
  //     has been executed and has indicated that its filter is passed. This
  //     provisional acceptance can be overridden by the other lists.
  //  c. The event is rejected unless all Algorithms in the required list have
  //     been executed and have indicated that their filter passed.
  //  d. The event is rejected if any Algorithm in the veto list has been
  //     executed and has indicated that its filter has passed.

  // Use IDecisionSvc, FilteredAlgorithm is a wrapper around DecisionSvc
  ATH_CHECK(m_decSvc.retrieve());

  // Register stream, no matter what Properties it has
  if (!m_decSvc->addStream(this->name()).isSuccess()) {
    ATH_MSG_ERROR("Couldn't add stream {}", this->name());
  }

  // Propagate the FilteredAlgorithm's Properties to IDecisionSvc
  for (const std::string& alg : m_acceptNames.value()) {
    if (!m_decSvc->addAcceptAlg(alg, this->name()).isSuccess()) {
      ATH_MSG_ERROR("Could not add '{}' to AcceptAlg list", alg);
    }
  }

  for (const std::string& alg : m_requireNames.value()) {
    if (!m_decSvc->addRequireAlg(alg, this->name()).isSuccess()) {
      ATH_MSG_ERROR("Could not add '{}' to RequireAlg list", alg);
    }
  }

  for (const std::string& alg : m_vetoNames.value()) {
    if (!m_decSvc->addVetoAlg(alg, this->name()).isSuccess()) {
      ATH_MSG_ERROR("Could not add '{}' to VetoAlg list", alg);
    }
  }

  return StatusCode::SUCCESS;
}


bool
FilteredAlgorithm::isEventAccepted(const EventContext& ctx) const
{
  return m_decSvc->isEventAccepted(this->name(), ctx);
}
