/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SubjetBuilderAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODJet/Jet.h"
#include "xAODBase/IParticle.h"

#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"

namespace ftag {

  StatusCode SubjetBuilderAlg::initialize() {
    ATH_CHECK(m_targetJetsKey.initialize());
    ATH_CHECK(m_ghostKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    ATH_CHECK(m_linkKey.initialize());
    ATH_CHECK(m_countKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode SubjetBuilderAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::JetContainer> targetJetsH(m_targetJetsKey, ctx);
    if (!targetJetsH.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container: "
                    << m_targetJetsKey.key());
      return StatusCode::FAILURE;
    }

    // Recorded up front so the event always gets a container, and subjets are
    // only created inside one that owns them.
    SG::WriteHandle<xAOD::JetContainer> outputH(m_outputKey, ctx);
    ATH_CHECK(outputH.record(std::make_unique<xAOD::JetContainer>(),
                             std::make_unique<xAOD::JetAuxContainer>()));

    SG::ReadDecorHandle<xAOD::JetContainer, IPLV> ghosts(m_ghostKey, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, IPLV> linkDec(m_linkKey, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, int> countDec(m_countKey, ctx);

    const fastjet::JetDefinition jetdef(fastjet::antikt_algorithm, m_radius);

    for (const xAOD::Jet* jet : *targetJetsH) {

      const IPLV& constituents = ghosts(*jet);
      std::vector<fastjet::PseudoJet> inputs;
      inputs.reserve(constituents.size());
      for (const ElementLink<xAOD::IParticleContainer>& link : constituents) {
        if (!link.isValid()) {
          ATH_MSG_ERROR("Invalid link in ghost association '"
                        << m_ghostKey.key()
                        << "'; the constituent was thinned away.");
          return StatusCode::FAILURE;
        }
        const xAOD::IParticle& part = **link;
        const auto p4 = part.p4();
        inputs.emplace_back(p4.Px(), p4.Py(), p4.Pz(), p4.E());
      }

      if (inputs.empty()) {
        linkDec(*jet) = {};
        countDec(*jet) = 0;
        continue;
      }

      const fastjet::ClusterSequence cs(inputs, jetdef);
      const auto subjetsPj = fastjet::sorted_by_pt(cs.inclusive_jets(m_ptMin));

      IPLV links;
      links.reserve(subjetsPj.size());
      for (const auto& pj : subjetsPj) {
        // Owned from the moment it exists.
        xAOD::Jet& subjet = *outputH->emplace_back(new xAOD::Jet());
        xAOD::JetFourMom_t p4;
        p4.SetPxPyPzE(pj.px(), pj.py(), pj.pz(), pj.e());
        subjet.setJetP4(p4);

        links.emplace_back(*outputH, outputH->size() - 1, ctx);
      }

      countDec(*jet) = static_cast<int>(links.size());
      linkDec(*jet) = std::move(links);
    }

    return StatusCode::SUCCESS;
  }

} // end namespace ftag
