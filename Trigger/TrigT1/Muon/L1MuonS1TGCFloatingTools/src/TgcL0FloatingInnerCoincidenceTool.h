/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGINNERCOINCIDENCETOOL_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGINNERCOINCIDENCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/ITgcL0InnerCoincidenceTool.h"

namespace L1Muon {

class TgcL0FloatingInnerCoincidenceTool final
    : public extends<AthAlgTool, ITgcL0InnerCoincidenceTool> {
 public:
  using base_class::base_class;

  /// \copydoc ITgcL0InnerCoincidenceTool::apply
  StatusCode apply(TgcL0CandidateContainer& candidates,
                   const EventContext& ctx) const override;
};

}  // namespace L1Muon

#endif
