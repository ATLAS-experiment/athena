/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/versions/NSWCandDataAuxContainer_v1.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"

namespace {
  static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD {
  NSWCandDataAuxContainer_v1::NSWCandDataAuxContainer_v1()
    : AuxContainerBase() {

    // NSW trigger data
    PRD_AUXVARIABLE(bcid);
    PRD_AUXVARIABLE(nSegments);
    PRD_AUXVARIABLE(overflow);
    PRD_AUXVARIABLE(fiberId);
    PRD_AUXVARIABLE(boardId);
    AUX_VARIABLE(segmentWord);
  }
}
