/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCBITWISETOOLS_BITWISECANDIDATEBUILDERTOOL_H
#define L1MUONS1TGCBITWISETOOLS_BITWISECANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/ICandBuilderTool.h"

namespace L1Muon::L1TGC {

class BitwiseCandidateBuilderTool final
    : public extends<AthAlgTool, ICandBuilderTool> {
 public:
  using ICandBuilderTool::build;

  using base_class::base_class;

  /// \copydoc ICandBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcSLCandidateContainer& candidates,
                   const EventContext& ctx) const override;
};

}  // namespace L1Muon::L1TGC

#endif
