/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jackson Burzynski

//
// includes
//
#include <JetAnalysisAlgorithms/JetGhostMergingAlg.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>

namespace CP
{
  StatusCode JetGhostMergingAlg ::
  initialize ()
  {
    // containers
    ATH_CHECK (m_jetLocation.initialize ());

    // decorators
    ATH_CHECK (m_mergedGhostContainer.initialize ());

    // accessors
    m_ghostTrackKeys.reserve(m_inputGhostTrackNames.size());
    for (const auto& ghostName: m_inputGhostTrackNames) {
      m_ghostTrackKeys.emplace_back( this, ghostName, m_jetLocation, ghostName, "");
      ATH_CHECK( m_ghostTrackKeys.back().initialize() );
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetGhostMergingAlg ::
  execute (const EventContext &ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> inputJets(m_jetLocation,ctx);
    if (!inputJets.isValid()) {
        ATH_MSG_FATAL("No jet collection with name " << m_jetLocation.key() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }

    // define GhostTrackContainer type (GTC)
    using GTC = std::vector<ElementLink<DataVector<xAOD::IParticle>>>;

    // create the accessors for each ghost track collection
    std::vector<SG::ReadDecorHandle<xAOD::JetContainer, GTC>> ghostTrackAccs;
    ghostTrackAccs.reserve(m_ghostTrackKeys.size());
    for (const auto &key: m_ghostTrackKeys) {
      ghostTrackAccs.emplace_back(key, ctx);
    }

    SG::WriteDecorHandle<xAOD::JetContainer, GTC > mergedGhostDecor(m_mergedGhostContainer, ctx);

    for(const xAOD::Jet *jet: *inputJets) {
        std::vector<ElementLink<xAOD::IParticleContainer>> mergedGhosts;
        std::size_t nGhosts = 0;
        for (const auto& ghostTrackAcc: ghostTrackAccs) {
            nGhosts += ghostTrackAcc( *jet ).size();
        }
        mergedGhosts.reserve(nGhosts);
        for (const auto& ghostTrackAcc: ghostTrackAccs) {
            const GTC &ghosts = ghostTrackAcc( *jet );
            mergedGhosts.insert(mergedGhosts.end(), ghosts.begin(), ghosts.end());
        }
        mergedGhostDecor(*jet) = std::move(mergedGhosts);
    }
    return StatusCode::SUCCESS;
  }

}
