/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCBITWISETOOLS_BITWISETRACKSELECTORTOOL_H
#define L1MUONS1TGCBITWISETOOLS_BITWISETRACKSELECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/ITrackSelectorTool.h"

namespace L1Muon::L1TGC {

class BitwiseTrackSelectorTool final
    : public extends<AthAlgTool, ITrackSelectorTool> {
 public:
  using base_class::base_class;

  /// \copydoc ITrackSelectorTool::select
  StatusCode select(const TgcSLCandidateContainer& candidates,
                    xAOD::L1TGCCandDataContainer& output,
                    const EventContext& ctx) const override;
};

}  // namespace L1Muon::L1TGC

#endif
