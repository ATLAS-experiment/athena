/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_IVALIDATIONBRANCHTOOL_H
#define L1MUONVALIDATION_IVALIDATIONBRANCHTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/StatusCode.h"

namespace MuonVal {
class MuonTesterTree;
}

namespace L1Muon {

/** @brief Common extension interface for Muon-trigger validation content.
 *
 * Implementations add subsystem-specific MuonTester branches to a common tree.
 * The common validation algorithm does not need to know which subsystem owns
 * the branches.  Matching and performance calculations are intentionally not
 * part of this interface and belong to downstream ROOT analysis.
 */
class IValidationBranchTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IValidationBranchTool, 1, 0);
  virtual ~IValidationBranchTool() override = default;

  /** @brief Register all branches owned by this validation component. */
  virtual StatusCode addBranches(MuonVal::MuonTesterTree& tree) = 0;
};

}  // namespace L1Muon

#endif
