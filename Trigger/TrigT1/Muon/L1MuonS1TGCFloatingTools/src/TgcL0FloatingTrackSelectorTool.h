/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGTRACKSELECTORTOOL_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGTRACKSELECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/ITrackSelectorTool.h"

namespace L1Muon {

class TgcL0FloatingTrackSelectorTool final
    : public extends<AthAlgTool, L1TGC::ITrackSelectorTool> {
 public:
  using base_class::base_class;

  /// \copydoc L1TGC::ITrackSelectorTool::select
  StatusCode select(const TgcSLCandidateContainer& candidates,
                    xAOD::L1TGCCandDataContainer& output,
                    const EventContext& ctx) const override;
};

}  // namespace L1Muon

#endif
