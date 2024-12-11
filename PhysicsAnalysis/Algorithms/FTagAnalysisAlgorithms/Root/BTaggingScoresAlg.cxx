/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Diego Baron

#include "FTagAnalysisAlgorithms/BTaggingScoresAlg.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

namespace CP {

  StatusCode BTaggingScoresAlg::initialize()
  {

    // Input handles
    ANA_CHECK(m_jetsKey.initialize());

    // Output decorations
    for(const std::string& var : m_vars) m_accdecs.emplace_back(var, var);

    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingScoresAlg::execute()
  {
    const EventContext &ctx = Gaudi::Hive::currentContext();

    // retrieve objects
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey, ctx);
    ANA_CHECK(jets.isValid());

    for(const xAOD::Jet* jet : *jets) {
        const xAOD::BTagging *btag = xAOD::BTaggingUtilities::getBTagging( *jet );

        // copy the values from b-tagging onto the jet
        for(const auto&[acc, dec] : m_accdecs) dec(*jet) = acc(*btag);
    }
    return StatusCode::SUCCESS;
  }

} // namespace
