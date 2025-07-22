/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxiliaryMeasurement/versions/AuxiliaryMeasurementAuxContainer1D_v1.h"
namespace xAOD{
    AuxiliaryMeasurementAuxContainer1D_v1::AuxiliaryMeasurementAuxContainer1D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 1);
        AUX_MEASUREMENTVAR(localPosition, 1);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
