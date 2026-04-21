/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/versions/StripClusterAuxContainer_v1.h"

namespace xAOD {
StripClusterAuxContainer_v1::StripClusterAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(identifierHash);
    AUX_VARIABLE(identifier);
    AUX_MEASUREMENTVAR(localPosition, 1);
    AUX_MEASUREMENTVAR(localCovariance, 1);
    AUX_VARIABLE(globalPosition);
    AUX_VARIABLE(channelsInPhi);
}
}  // namespace xAOD
