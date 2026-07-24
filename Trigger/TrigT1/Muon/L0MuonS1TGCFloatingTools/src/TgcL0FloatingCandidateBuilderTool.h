/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGCANDIDATEBUILDERTOOL_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGCANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0CandidateBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace L0Muon {

class TgcL0FloatingCandidateBuilderTool final
    : public extends<AthAlgTool, ITgcL0CandidateBuilderTool> {
 public:
  using base_class::base_class;

  StatusCode initialize() override;

  /// \copydoc ITgcL0CandidateBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcL0CandidateContainer& candidates,
                   const EventContext& ctx) const override;

 private:
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{
      this, "CablingKey", "MuonTgc_CablingMap"};
};

}  // namespace L0Muon

#endif
