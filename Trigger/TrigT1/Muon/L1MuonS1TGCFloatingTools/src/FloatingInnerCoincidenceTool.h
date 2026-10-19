/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGINNERCOINCIDENCETOOL_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGINNERCOINCIDENCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/IInnerCoincidenceTool.h"

namespace L1Muon::L1TGC {

class FloatingInnerCoincidenceTool final
    : public extends<AthAlgTool, IInnerCoincidenceTool> {
 public:
  using base_class::base_class;

  /// \copydoc IInnerCoincidenceTool::apply
  StatusCode apply(TgcSLCandidateContainer& candidates,
                   const EventContext& ctx) const override;
};

}  // namespace L1Muon::L1TGC

#endif
