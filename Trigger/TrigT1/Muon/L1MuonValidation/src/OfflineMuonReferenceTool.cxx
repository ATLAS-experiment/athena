/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OfflineMuonReferenceTool.h"

#include "StoreGate/ReadHandle.h"

#include <cstddef>
#include <unordered_set>

namespace L1Muon {

OfflineMuonReferenceTool::OfflineMuonReferenceTool(const std::string& type,
                                                   const std::string& name,
                                                   const IInterface* parent)
    : AthAlgTool(type, name, parent) {
  declareInterface<IOfflineMuonReferenceTool>(this);
}

StatusCode OfflineMuonReferenceTool::initialize() {
  ATH_CHECK(m_muonKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode OfflineMuonReferenceTool::muons(
    const EventContext& ctx,
    std::vector<const xAOD::Muon*>& references) const {
  references.clear();

  SG::ReadHandle<xAOD::MuonContainer> muons{m_muonKey, ctx};
  if (!muons.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_muonKey.fullKey());
    return StatusCode::FAILURE;
  }

  references.reserve(muons->size());
  std::unordered_set<const xAOD::Muon*> seen{};
  seen.reserve(muons->size());
  for (std::size_t index = 0; index < muons->size(); ++index) {
    const xAOD::Muon* muon = (*muons)[index];
    if (muon == nullptr) {
      ATH_MSG_ERROR("Null offline muon at index " << index << " in "
                    << m_muonKey.fullKey());
      references.clear();
      return StatusCode::FAILURE;
    }
    if (!seen.insert(muon).second) {
      ATH_MSG_ERROR("Duplicate offline-muon pointer at index " << index
                    << " in " << m_muonKey.fullKey());
      references.clear();
      return StatusCode::FAILURE;
    }
    references.push_back(muon);
  }
  return StatusCode::SUCCESS;
}

std::string OfflineMuonReferenceTool::inputKey() const {
  return m_muonKey.key();
}

}  // namespace L1Muon
