/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetLinkMatcherAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "xAODJet/JetContainer.h"
#include <vector>
#include <memory>

namespace ftag {

  JetLinkMatcherAlg::JetLinkMatcherAlg(const std::string& name,
                               ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator)
  {
    declareProperty("floatsToCopy", m_floats.toCopy);
    declareProperty("doublesToCopy", m_doubles.toCopy);
    declareProperty("intsToCopy", m_ints.toCopy);
    declareProperty("uintsToCopy", m_uints.toCopy);
    declareProperty("ulongsToCopy", m_ulongs.toCopy);
    declareProperty("charsToCopy", m_chars.toCopy);
    declareProperty("charVectorsToCopy", m_charVectors.toCopy);
    declareProperty("iparticlesToCopy", m_iparticles.toCopy);
    declareProperty("trackLinksToCopy", m_trackLinks.toCopy);
  }

  StatusCode JetLinkMatcherAlg::initialize() {
    std::vector<std::string> sources;
    for (const auto& key: m_sourceJets) {
      sources.emplace_back(key.key());
    }
    std::string target = m_targetJet.key();
    ATH_CHECK(m_floats.initialize(this, sources, target));
    ATH_CHECK(m_doubles.initialize(this, sources, target));
    ATH_CHECK(m_ints.initialize(this, sources, target));
    ATH_CHECK(m_uints.initialize(this, sources, target));
    ATH_CHECK(m_ulongs.initialize(this, sources, target));
    ATH_CHECK(m_chars.initialize(this, sources, target));
    ATH_CHECK(m_charVectors.initialize(this, sources, target));
    ATH_CHECK(m_iparticles.initialize(this, sources, target));
    ATH_CHECK(m_trackLinks.initialize(this, sources, target));
    ATH_CHECK(m_targetJet.initialize());
    ATH_CHECK(m_sourceJets.initialize());
    ATH_CHECK(m_link.initialize());
    if (!m_matchDecorator.empty()) {
      ATH_CHECK(m_matchDecorator.initialize());
    }
    return StatusCode::SUCCESS;
  }

  StatusCode JetLinkMatcherAlg::execute(const EventContext& cxt) const {
    SG::ReadHandle<JC> targetJetGet(m_targetJet, cxt);
    SG::ReadDecorHandle<JC, ElementLink<JC>> link(m_link, cxt);
    std::optional<SG::WriteDecorHandle<JC,char>> matchDecorator;
    if (!m_matchDecorator.empty()) {
      matchDecorator.emplace(m_matchDecorator, cxt);
    }
    std::vector<MatchedPair<JC>> matches;
    for (const xAOD::Jet* target: *targetJetGet) {
      const ElementLink<JC>& sourceLink = link(*target);
      const xAOD::Jet* source = sourceLink.isValid() ?
        *sourceLink : nullptr;
      if (!matchDecorator && !source) {
        throw std::runtime_error("invalid link to source");
      }
      if (source) {
        if (matchDecorator) matchDecorator.value()(*target) = 1;
        matches.push_back({source, target});
      } else {
        matchDecorator.value()(*target) = 0;
        matches.push_back({nullptr, target});
      }
    }
    m_floats.copy(matches, cxt);
    m_doubles.copy(matches, cxt);
    m_ints.copy(matches, cxt);
    m_uints.copy(matches, cxt);
    m_ulongs.copy(matches, cxt);
    m_chars.copy(matches, cxt);
    m_charVectors.copy(matches, cxt);
    m_iparticles.copy(matches, cxt);
    m_trackLinks.copy(matches, cxt);

    return StatusCode::SUCCESS;
  }
  StatusCode JetLinkMatcherAlg::finalize () {
    return StatusCode::SUCCESS;
  }

}// end namespace ftag
