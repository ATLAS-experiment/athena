/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DecisionCollectorTool.h"


StatusCode DecisionCollectorTool::initialize() {
  ATH_CHECK( m_decisionsKey.initialize() );
  renounceArray( m_decisionsKey ); 
  return StatusCode::SUCCESS;
}

void DecisionCollectorTool::getSequencesNames( std::set<std::string>& output ) const {
  for (const auto& decisionKey: m_decisionsKey) {
    output.insert(decisionKey.key());
  }
}

namespace {
  // Helper to avoid code duplication
  void fillDecisions(SG::ReadHandle<TrigCompositeUtils::DecisionContainer>& handle,
                     std::vector<TrigCompositeUtils::DecisionID>& output) {
    for ( const TrigCompositeUtils::Decision* d : *handle.cptr() )  {
      output.insert( output.end(),
                     TrigCompositeUtils::decisionIDs( d ).begin(),
                     TrigCompositeUtils::decisionIDs( d ).end() );
    }
  }
}

void DecisionCollectorTool::getDecisions( std::vector<TrigCompositeUtils::DecisionID>& output,
                                          const EventContext& ctx ) const {
  for (const auto& decisionKey: m_decisionsKey ) {
    auto handle = SG::makeHandle( decisionKey, ctx );
    if ( handle.isValid() ) {
      fillDecisions(handle, output);
    }
  }
}

void DecisionCollectorTool::getDecisions( std::vector<TrigCompositeUtils::DecisionID>& decisions,
                                          std::set<std::string>& sequences,
                                          const EventContext& ctx ) const {
  for (const auto& decisionKey: m_decisionsKey ) {
    auto handle = SG::makeHandle( decisionKey, ctx );
    if ( handle.isValid() ) {
      fillDecisions(handle, decisions);
      sequences.insert(decisionKey.key());
    }
  }
}
