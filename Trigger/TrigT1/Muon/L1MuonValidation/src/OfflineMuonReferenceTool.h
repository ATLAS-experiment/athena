/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_OFFLINEMUONREFERENCETOOL_H
#define L1MUONVALIDATION_OFFLINEMUONREFERENCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonValidation/IOfflineMuonReferenceTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuon/MuonContainer.h"

#include <string>

namespace L1Muon {

/** @brief Common provider for reconstructed offline muon references. */
class OfflineMuonReferenceTool final
    : public AthAlgTool,
      virtual public IOfflineMuonReferenceTool {
 public:
  OfflineMuonReferenceTool(const std::string& type, const std::string& name,
                           const IInterface* parent);

  StatusCode initialize() override;
  StatusCode muons(
      const EventContext& ctx,
      std::vector<const xAOD::Muon*>& references) const override;
  std::string inputKey() const override;

 private:
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{
      this, "MuonKey", "Muons", "Input reconstructed offline-muon collection"};
};

}  // namespace L1Muon

#endif
