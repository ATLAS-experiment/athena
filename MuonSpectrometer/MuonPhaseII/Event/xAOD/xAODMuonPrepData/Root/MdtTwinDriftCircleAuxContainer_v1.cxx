/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/MdtTwinDriftCircleAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"Mdt_"};
}

namespace xAOD {
MdtTwinDriftCircleAuxContainer_v1::MdtTwinDriftCircleAuxContainer_v1()
    : AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
  
    AUX_MEASUREMENTVAR(localPosition, 2)
    AUX_MEASUREMENTVAR(localCovariance, 2)
    
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(tdc);
    PRD_AUXVARIABLE(adc);
    PRD_AUXVARIABLE(driftTube);
    PRD_AUXVARIABLE(tubeLayer);
    PRD_AUXVARIABLE(status);

    PRD_AUXVARIABLE(twinTdc);
    PRD_AUXVARIABLE(twinAdc);
    PRD_AUXVARIABLE(twinTube);
    PRD_AUXVARIABLE(twinLayer);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE