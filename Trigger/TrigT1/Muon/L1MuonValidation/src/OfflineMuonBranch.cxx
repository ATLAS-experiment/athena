/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OfflineMuonBranch.h"

#include <cstdint>
#include <vector>

namespace L1Muon {

OfflineMuonBranch::OfflineMuonBranch(
    MuonVal::MuonTesterTree& tree,
    const IOfflineMuonReferenceTool* referenceTool,
    const MSG::Level msgLevel)
    : MuonVal::MuonTesterBranch(tree, " offlineMuon module"),
      m_referenceTool{referenceTool},
      m_muonDependencyKey{referenceTool != nullptr ? referenceTool->inputKey()
                                                   : std::string{}},
      m_fourMom{std::make_shared<MuonVal::IParticleFourMomBranch>(
          tree, "offlineMuon")} {
  setLevel(msgLevel);

  // These are native xAOD::Muon auxiliary fields.  Keep quality optional:
  // its decoration depends on the offline reconstruction/configuration.
  m_variablesConfigured &= m_fourMom->addVariable<std::uint16_t>("author");
  m_variablesConfigured &= m_fourMom->addVariable<std::uint16_t>(
      static_cast<std::uint16_t>(0U), "allAuthors");
  m_variablesConfigured &= m_fourMom->addVariable<std::uint16_t>("muonType");
  m_variablesConfigured &= m_fourMom->addVariable<std::uint8_t>(
      static_cast<std::uint8_t>(255U), "quality");
}

bool OfflineMuonBranch::init() {
  if (m_referenceTool == nullptr || m_muonDependencyKey.empty() ||
      !m_fourMom || !m_variablesConfigured) {
    ATH_MSG_ERROR("Offline-muon branch configuration is incomplete");
    return false;
  }
  return declare_dependency(m_muonDependencyKey) && m_fourMom->init();
}

bool OfflineMuonBranch::fill(const EventContext& ctx) {
  std::vector<const xAOD::Muon*> references{};
  if (m_referenceTool->muons(ctx, references).isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve offline-muon references");
    return false;
  }

  for (const xAOD::Muon* muon : references) {
    m_fourMom->push_back(muon);
  }
  return m_fourMom->fill(ctx);
}

}  // namespace L1Muon
