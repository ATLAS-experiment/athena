/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxiliaryMeasurement/versions/AuxiliaryMeasurementAuxContainer2D_v1.h"
namespace xAOD{
    AuxiliaryMeasurementAuxContainer2D_v1::AuxiliaryMeasurementAuxContainer2D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 2);
        AUX_MEASUREMENTVAR(localPosition, 2);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
