/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/RpcStrip2DAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"Rpc_"};
}

namespace xAOD {
RpcStrip2DAuxContainer_v1::RpcStrip2DAuxContainer_v1()
    : AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);  
    AUX_MEASUREMENTVAR(localPosition, 2)
    AUX_MEASUREMENTVAR(localCovariance, 2)
    
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(time);
    PRD_AUXVARIABLE(timeCovariance);
    PRD_AUXVARIABLE(triggerInfo);
    PRD_AUXVARIABLE(ambiguityFlag);
    PRD_AUXVARIABLE(timeOverThreshold);

    PRD_AUXVARIABLE(stripNumber);
    PRD_AUXVARIABLE(gasGap);
    PRD_AUXVARIABLE(doubletPhi);
}
}  // namespace xAOD
