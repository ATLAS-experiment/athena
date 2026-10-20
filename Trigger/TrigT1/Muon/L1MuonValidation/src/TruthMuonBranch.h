/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_TRUTHMUONBRANCH_H
#define L1MUONVALIDATION_TRUTHMUONBRANCH_H

#include "GeneratorObjects/McEventCollection.h"
#include "L1MuonValidation/ITruthMuonReferenceTool.h"
#include "MuonTesterTree/MuonTesterBranch.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "StoreGate/ReadHandleKey.h"

namespace L1Muon {

/** @brief Common generator-truth muon branch for muon-trigger validation.
 *
 * Selection and ordering are owned by ITruthMuonReferenceTool and shared with
 * detector-specific extrapolation branches.  Only subsystem-independent truth
 * quantities are written here.
 */
class TruthMuonBranch final : public MuonVal::MuonTesterBranch {
 public:
  TruthMuonBranch(MuonVal::MuonTesterTree& tree,
                  const ITruthMuonReferenceTool* referenceTool,
                  MSG::Level msgLevel = MSG::Level::INFO);

  bool init() override;
  bool fill(const EventContext& ctx) override;

 private:
  const ITruthMuonReferenceTool* m_referenceTool{nullptr};
  SG::ReadHandleKey<McEventCollection> m_truthDependencyKey{};

  MuonVal::VectorBranch<int>& m_pdgId{
      parent().newVector<int>("truthMuon_pdgId")};
  MuonVal::VectorBranch<int>& m_barcode{
      parent().newVector<int>("truthMuon_barcode")};
  MuonVal::VectorBranch<float>& m_pt{
      parent().newVector<float>("truthMuon_pt")};
  MuonVal::VectorBranch<float>& m_eta{
      parent().newVector<float>("truthMuon_eta")};
  MuonVal::VectorBranch<float>& m_phi{
      parent().newVector<float>("truthMuon_phi")};
  MuonVal::VectorBranch<int>& m_charge{
      parent().newVector<int>("truthMuon_charge")};
};

}  // namespace L1Muon

#endif
