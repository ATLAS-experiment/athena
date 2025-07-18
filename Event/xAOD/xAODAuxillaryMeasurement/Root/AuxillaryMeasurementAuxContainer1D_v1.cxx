/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurementAuxContainer1D_v1.h"
namespace xAOD{
    AuxillaryMeasurementAuxContainer1D_v1::AuxillaryMeasurementAuxContainer1D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 1);
        AUX_MEASUREMENTVAR(localPosition, 1);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
