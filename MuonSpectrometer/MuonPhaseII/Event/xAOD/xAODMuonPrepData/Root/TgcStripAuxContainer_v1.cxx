/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/TgcStripAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"Tgc_"};
}

namespace xAOD {
TgcStripAuxContainer_v1::TgcStripAuxContainer_v1()
    : AuxContainerBase() {
    /// Identifier variable hopefully unique
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
  
    AUX_MEASUREMENTVAR(localPosition, 1)
    AUX_MEASUREMENTVAR(localCovariance, 1)
    
    /// Names may be shared across different subdetectors
    PRD_AUXVARIABLE(bcBitMap);

    PRD_AUXVARIABLE(channelNumber);
    PRD_AUXVARIABLE(gasGap);
    PRD_AUXVARIABLE(measuresPhi);
    PRD_AUXVARIABLE(stripPosInStation);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE