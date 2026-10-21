/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_OFFLINEMUONBRANCHTOOL_H
#define L1MUONVALIDATION_OFFLINEMUONBRANCHTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "L1MuonValidation/IOfflineMuonReferenceTool.h"
#include "L1MuonValidation/IValidationBranchTool.h"

#include <string>

namespace L1Muon {

/** @brief Common branch provider for reconstructed offline muons. */
class OfflineMuonBranchTool final : public AthAlgTool,
                                    virtual public IValidationBranchTool {
 public:
  OfflineMuonBranchTool(const std::string& type, const std::string& name,
                        const IInterface* parent);

  StatusCode initialize() override;
  StatusCode addBranches(MuonVal::MuonTesterTree& tree) override;

 private:
  ToolHandle<IOfflineMuonReferenceTool> m_referenceTool{
      this, "OfflineMuonReferenceTool", "",
      "Common reconstructed offline-muon reference provider"};
};

}  // namespace L1Muon

#endif
