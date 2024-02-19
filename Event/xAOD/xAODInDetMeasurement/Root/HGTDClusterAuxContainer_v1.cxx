/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/versions/HGTDClusterAuxContainer_v1.h"

namespace xAOD {
HGTDClusterAuxContainer_v1::HGTDClusterAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
    AUX_MEASUREMENTVAR(localPosition, 3);
    AUX_MEASUREMENTVAR(localCovariance, 3);
    AUX_VARIABLE(rdoList);
    AUX_VARIABLE(totList);
}
}  // namespace xAOD
