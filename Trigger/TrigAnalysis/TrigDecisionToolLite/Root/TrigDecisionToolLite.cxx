/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigDecisionToolLite/TrigDecisionToolLite.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigAnalysisHelpers/FeatureRequestHelpers.h"

#include "AsgMessaging/MsgStream.h"

std::atomic<bool> Trig::TrigDecisionToolLite::s_printWarningMessages = true;

Trig::TrigDecisionToolLite::TrigDecisionToolLite(const std::string& name): asg::AsgTool(name) {}

StatusCode Trig::TrigDecisionToolLite::initialize() {
  ATH_CHECK(m_HLTSummaryKeyIn.initialize());
  return StatusCode::SUCCESS;
}

StatusCode Trig::TrigDecisionToolLite::fillPassingChainsSet(TrigCompositeUtils::DecisionIDContainer& setOfAllPassingChains, const EventContext& ctx) const {
  SG::ReadHandle<TrigCompositeUtils::DecisionContainer> navigationRH = SG::ReadHandle(m_HLTSummaryKeyIn, ctx);
  if ( not navigationRH.isValid() ) {
    ATH_MSG_ERROR("Cannot read trigger navigation container " << m_HLTSummaryKeyIn.key());
    return StatusCode::FAILURE;
  }
  const TrigCompositeUtils::Decision* terminusNode = TrigCompositeUtils::getTerminusNode(navigationRH);
  if ( not terminusNode ) {
    ATH_MSG_ERROR("Cannot locate terminus node within " << m_HLTSummaryKeyIn.key() << ", the navigation graph has " << navigationRH->size() << " nodes.");
    return StatusCode::FAILURE;
  }
  TrigCompositeUtils::decisionIDs(terminusNode, setOfAllPassingChains); // Populate setOfAllPassingChains from the vector of passing chain IDs in terminusNode
  return StatusCode::SUCCESS;
}

bool Trig::TrigDecisionToolLite::isPassed(const std::vector<HLT::Identifier>& chainIDs, const EventContext& ctx) const {
  TrigCompositeUtils::DecisionIDContainer setOfAllPassingChains;
  if ( fillPassingChainsSet(setOfAllPassingChains, ctx).isFailure() ) {
    return false;
  }
  for ( const HLT::Identifier& chainID : chainIDs ) { // Return the OR of the vector of supplied chains
    if ( setOfAllPassingChains.count( chainID.numeric() ) ) {
      return true;
    }
  }
  return false;
}

bool Trig::TrigDecisionToolLite::isPassed(const HLT::Identifier& chainID, const EventContext& ctx) const {
  TrigCompositeUtils::DecisionIDContainer setOfAllPassingChains;
  if ( fillPassingChainsSet(setOfAllPassingChains, ctx).isFailure() ) {
    return false;
  }
  return setOfAllPassingChains.count( chainID.numeric() );
}

bool Trig::TrigDecisionToolLite::isPassed(const std::vector<std::string>& chains, const EventContext& ctx) const {
  std::vector<HLT::Identifier> chainIDs;
  for ( const std::string& chain : chains ) {
    chainIDs.emplace_back(chain);
  }
  return isPassed( chainIDs, ctx );
}

bool Trig::TrigDecisionToolLite::isPassed(const std::string& chain, const EventContext& ctx) const {
  return isPassed( HLT::Identifier(chain), ctx );
}

std::vector<TrigCompositeUtils::TypelessLinkInfo>
Trig::TrigDecisionToolLite::typelessFeatures(const Trig::FeatureRequestDescriptor& frd, const CLID clid, const EventContext& ctx) const {
  return FeatureRequestHelpers::typelessFeaturesImplimentation(frd, clid, m_HLTSummaryKeyIn, msg(), ctx, getEventStore(), s_printWarningMessages);
}

const asg::EventStoreType* Trig::TrigDecisionToolLite::getEventStore() const {
  return &*evtStore(); // Service handle to pointer (in Athena)
}
