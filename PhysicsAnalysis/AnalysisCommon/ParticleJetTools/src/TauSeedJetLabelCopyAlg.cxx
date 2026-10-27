/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TauSeedJetLabelCopyAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <limits>

namespace ParticleJetTools {

TauSeedJetLabelCopyAlg::TauSeedJetLabelCopyAlg(const std::string& name, ISvcLocator* loc)
  : AthReentrantAlgorithm(name, loc) {}

StatusCode TauSeedJetLabelCopyAlg::initialize() {
  ATH_CHECK(m_tauKey.initialize());
  ATH_CHECK(m_jetKey.initialize());
  if (!m_linkName.empty()) {
    m_linkAcc = std::make_unique<SG::ConstAccessor<ElementLink<xAOD::JetContainer>>>(m_linkName);
  }

  const std::string tauPrefix = m_tauKey.key() + ".";
  for (const std::string& name : m_intNames) {
    m_intAccs.emplace_back(name);
    m_intDecorKeys.emplace_back(tauPrefix + name);
  }
  for (const std::string& name : m_floatNames) {
    m_floatAccs.emplace_back(name);
    m_floatDecorKeys.emplace_back(tauPrefix + name);
  }
  ATH_CHECK(m_intDecorKeys.initialize());
  ATH_CHECK(m_floatDecorKeys.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TauSeedJetLabelCopyAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<xAOD::TauJetContainer> taus(m_tauKey, ctx);
  SG::ReadHandle<xAOD::JetContainer> seedJets(m_jetKey, ctx);
  ATH_CHECK(taus.isValid());
  ATH_CHECK(seedJets.isValid());
  if (!m_linkAcc && seedJets->size() != taus->size()) {
    ATH_MSG_ERROR(taus->size() << " taus but " << seedJets->size() << " seed jets to match by index");
    return StatusCode::FAILURE;
  }

  auto intHandles = m_intDecorKeys.makeHandles(ctx);
  auto floatHandles = m_floatDecorKeys.makeHandles(ctx);

  for (std::size_t index = 0; index < taus->size(); ++index) {
    const xAOD::TauJet* tau = taus->at(index);
    const xAOD::Jet* jet = nullptr;
    if (m_linkAcc) {
      const ElementLink<xAOD::JetContainer>& link = (*m_linkAcc)(*tau);
      if (link.isValid()) jet = *link;
    } else {
      jet = seedJets->at(index);
    }
    for (std::size_t i = 0; i < intHandles.size(); ++i) {
      intHandles.at(i)(*tau) = jet ? m_intAccs.at(i)(*jet) : -1;
    }
    for (std::size_t i = 0; i < floatHandles.size(); ++i) {
      floatHandles.at(i)(*tau) = jet ? m_floatAccs.at(i)(*jet) : std::numeric_limits<float>::quiet_NaN();
    }
  }

  return StatusCode::SUCCESS;
}

}
