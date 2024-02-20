/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/MMClusterAuxContainer_v1.h"

namespace {
   static const std::string preFixStr{"Mm_"};
}
namespace xAOD {
MMClusterAuxContainer_v1::MMClusterAuxContainer_v1()
   : AuxContainerBase() {
   /// Identifier variable hopefully unique
   AUX_VARIABLE(identifier);
   AUX_VARIABLE(identifierHash);
   AUX_MEASUREMENTVAR(localPosition, 1);
   AUX_MEASUREMENTVAR(localCovariance, 1 );

   PRD_AUXVARIABLE(gasGap);
   PRD_AUXVARIABLE(channelNumber);
   /// Names may be shared across different subdetectors
   PRD_AUXVARIABLE(time);
   PRD_AUXVARIABLE(charge);
   PRD_AUXVARIABLE(driftDist);
   PRD_AUXVARIABLE(angle);
   PRD_AUXVARIABLE(chiSqProb);
   PRD_AUXVARIABLE(author);
   PRD_AUXVARIABLE(quality);
   PRD_AUXVARIABLE(stripNumbers);
   PRD_AUXVARIABLE(stripTimes);
   PRD_AUXVARIABLE(stripCharges);
   PRD_AUXVARIABLE(stripDriftDist);
   PRD_AUXVARIABLE(stripDriftErrors);
   PRD_AUXVARIABLE(stripPosInStation);
}
}  // namespace xAOD
#undef PRD_AUXVARIABLE