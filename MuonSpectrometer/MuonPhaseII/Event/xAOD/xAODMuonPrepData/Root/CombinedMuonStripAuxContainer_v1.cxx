/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODMuonPrepData/versions/AccessorMacros.h"
// Local include(s):
#include "xAODMuonPrepData/versions/CombinedMuonStripAuxContainer_v1.h"

namespace xAOD {
CombinedMuonStripAuxContainer_v1::CombinedMuonStripAuxContainer_v1()
    : AuxContainerBase() {
  AUX_VARIABLE(MuonStripLink1);
  AUX_VARIABLE(MuonStripLink2);
  AUX_MEASUREMENTVAR(localPosition, 2)
  AUX_MEASUREMENTVAR(localCovariance, 2)
}
}  // namespace xAOD
