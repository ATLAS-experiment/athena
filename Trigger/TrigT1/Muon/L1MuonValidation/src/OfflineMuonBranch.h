/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_OFFLINEMUONBRANCH_H
#define L1MUONVALIDATION_OFFLINEMUONBRANCH_H

#include "L1MuonValidation/IOfflineMuonReferenceTool.h"
#include "MuonTesterTree/IParticleFourMomBranch.h"
#include "MuonTesterTree/MuonTesterBranch.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuon/MuonContainer.h"

#include <memory>

namespace L1Muon {

/** @brief Common reconstructed-muon branch for muon-trigger validation.
 *
 * Four-momentum and charge are written by the existing MuonTester
 * IParticleFourMomBranch.  Author/type/quality information is appended using
 * the same helper.  No offline-muon selection or trigger matching is applied
 * in Athena.
 */
class OfflineMuonBranch final : public MuonVal::MuonTesterBranch {
 public:
  OfflineMuonBranch(MuonVal::MuonTesterTree& tree,
                    const IOfflineMuonReferenceTool* referenceTool,
                    MSG::Level msgLevel = MSG::Level::INFO);

  bool init() override;
  bool fill(const EventContext& ctx) override;

 private:
  const IOfflineMuonReferenceTool* m_referenceTool{nullptr};
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonDependencyKey{};
  std::shared_ptr<MuonVal::IParticleFourMomBranch> m_fourMom{};
  bool m_variablesConfigured{true};
};

}  // namespace L1Muon

#endif
