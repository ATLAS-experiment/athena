/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_MUONTRIGGERVALIDATIONALG_H
#define L1MUONVALIDATION_MUONTRIGGERVALIDATIONALG_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "L1MuonValidation/IValidationBranchTool.h"
#include "MuonTesterTree/MuonTesterTree.h"

namespace L1Muon {

/** @brief Common MuonTester ROOT producer for Phase-II muon-trigger validation.
 *
 * The algorithm owns only subsystem-independent infrastructure: the common
 * EventInfo block, one MuonTester tree, and a configurable list of branch
 * providers.  TGC/RPC/MDT/MUCTPI-specific content is supplied by tools through
 * IValidationBranchTool.
 *
 * No object matching, efficiencies, calibration fits, or performance decisions
 * are performed in Athena.
 */
class MuonTriggerValidationAlg final : public AthHistogramAlgorithm {
 public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) override;
  StatusCode finalize() override;
  unsigned int cardinality() const override final { return 1U; }

 private:
  Gaudi::Property<bool> m_isMC{
      this, "IsMC", true,
      "Input contains Monte Carlo information; enables MC EventInfo fields"};
  Gaudi::Property<bool> m_writeEventInfo{
      this, "WriteEventInfo", true,
      "Write the common MuonTester EventInfo block"};

  ToolHandleArray<IValidationBranchTool> m_branchTools{
      this, "BranchTools", {},
      "Subsystem-specific providers that add branches to the common tree"};

  MuonVal::MuonTesterTree m_tree{"MuonTrigger", "MUONTRIGGERVALID"};
};

}  // namespace L1Muon

#endif
