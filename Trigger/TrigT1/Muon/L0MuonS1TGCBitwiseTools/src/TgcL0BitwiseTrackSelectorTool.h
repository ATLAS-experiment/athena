/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCBITWISETOOLS_TGCL0BITWISETRACKSELECTORTOOL_H
#define L0MUONS1TGCBITWISETOOLS_TGCL0BITWISETRACKSELECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0TrackSelectorTool.h"

namespace L0Muon {

class TgcL0BitwiseTrackSelectorTool final
    : public extends<AthAlgTool, ITgcL0TrackSelectorTool> {
 public:
  using base_class::base_class;

  /// \copydoc ITgcL0TrackSelectorTool::select
  StatusCode select(const TgcL0CandidateContainer& candidates,
                    xAOD::TGCCandDataContainer& output,
                    const EventContext& ctx) const override;
};

}  // namespace L0Muon

#endif
