/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/versions/L1NSWCandDataAuxContainer_v1.h"
#include "xAODCore/tools/AuxVariable.h"

namespace xAOD {
  L1NSWCandDataAuxContainer_v1::L1NSWCandDataAuxContainer_v1()
    : AuxContainerBase() {

    // NSW trigger data
    AUX_VARIABLE(l1Bcid);
    AUX_VARIABLE(l1NSegments);
    AUX_VARIABLE(l1Overflow);
    AUX_VARIABLE(fiberID);
    AUX_VARIABLE(boardID);
    AUX_VARIABLE(l1SegmentWords);
  }
}
