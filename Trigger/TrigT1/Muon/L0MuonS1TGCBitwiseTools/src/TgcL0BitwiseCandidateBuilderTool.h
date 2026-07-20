/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCBITWISETOOLS_TGCL0BITWISECANDIDATEBUILDERTOOL_H
#define L0MUONS1TGCBITWISETOOLS_TGCL0BITWISECANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0CandidateBuilderTool.h"

namespace L0Muon {

class TgcL0BitwiseCandidateBuilderTool final
    : public extends<AthAlgTool, ITgcL0CandidateBuilderTool> {
 public:
  using base_class::base_class;

  /// \copydoc ITgcL0CandidateBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcL0CandidateContainer& candidates,
                   const EventContext& ctx) const override;
};

}  // namespace L0Muon

#endif
