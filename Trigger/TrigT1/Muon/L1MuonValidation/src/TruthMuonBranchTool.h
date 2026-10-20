/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_TRUTHMUONBRANCHTOOL_H
#define L1MUONVALIDATION_TRUTHMUONBRANCHTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "L1MuonValidation/ITruthMuonReferenceTool.h"
#include "L1MuonValidation/IValidationBranchTool.h"

#include <string>

namespace L1Muon {

/** @brief Common branch provider for selected generator-truth muons. */
class TruthMuonBranchTool final : public AthAlgTool,
                                  virtual public IValidationBranchTool {
 public:
  TruthMuonBranchTool(const std::string& type, const std::string& name,
                      const IInterface* parent);

  StatusCode initialize() override;
  StatusCode addBranches(MuonVal::MuonTesterTree& tree) override;

 private:
  ToolHandle<ITruthMuonReferenceTool> m_referenceTool{
      this, "TruthMuonReferenceTool", "",
      "Common selected truth-muon reference provider"};
};

}  // namespace L1Muon

#endif
