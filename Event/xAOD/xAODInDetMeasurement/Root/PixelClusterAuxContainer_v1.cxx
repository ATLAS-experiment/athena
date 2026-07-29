/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/versions/PixelClusterAuxContainer_v1.h"

namespace xAOD {
PixelClusterAuxContainer_v1::PixelClusterAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(identifier);
    AUX_VARIABLE(identifierHash);
    AUX_MEASUREMENTVAR(localPosition, 2);
    AUX_MEASUREMENTVAR(localCovariance, 2);
    AUX_VARIABLE(globalPosition);
    AUX_VARIABLE(channelsInPhi);
    AUX_VARIABLE(channelsInEta);
    AUX_VARIABLE(widthInEta);
    AUX_VARIABLE(totalCharge);
    AUX_VARIABLE(energyLoss);
    AUX_VARIABLE(lvl1a);
}
}  // namespace xAOD
