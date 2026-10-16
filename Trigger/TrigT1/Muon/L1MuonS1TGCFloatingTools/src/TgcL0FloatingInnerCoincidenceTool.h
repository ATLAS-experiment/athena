/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGINNERCOINCIDENCETOOL_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGINNERCOINCIDENCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/IInnerCoincidenceTool.h"

namespace L1Muon {

class TgcL0FloatingInnerCoincidenceTool final
    : public extends<AthAlgTool, L1TGC::IInnerCoincidenceTool> {
 public:
  using base_class::base_class;

  /// \copydoc L1TGC::IInnerCoincidenceTool::apply
  StatusCode apply(TgcSLCandidateContainer& candidates,
                   const EventContext& ctx) const override;
};

}  // namespace L1Muon

#endif
