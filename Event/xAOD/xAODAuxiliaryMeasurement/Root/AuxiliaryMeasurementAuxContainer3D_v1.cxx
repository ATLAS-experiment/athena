/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxiliaryMeasurement/versions/AuxiliaryMeasurementAuxContainer3D_v1.h"
namespace xAOD{
    AuxiliaryMeasurementAuxContainer3D_v1::AuxiliaryMeasurementAuxContainer3D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 3);
        AUX_MEASUREMENTVAR(localPosition, 3);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
