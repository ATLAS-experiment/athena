/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/sTgcWireAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"sTgc_"};
}

namespace xAOD {
sTgcWireAuxContainer_v1::sTgcWireAuxContainer_v1(): 
   AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
    AUX_MEASUREMENTVAR(localPosition, 1)
    AUX_MEASUREMENTVAR(localCovariance, 1)
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(author);
    PRD_AUXVARIABLE(gasGap);
    PRD_AUXVARIABLE(channelNumber);
    PRD_AUXVARIABLE(time);
    PRD_AUXVARIABLE(charge);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE