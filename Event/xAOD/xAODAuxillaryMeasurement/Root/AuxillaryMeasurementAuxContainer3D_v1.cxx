/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurementAuxContainer3D_v1.h"
namespace xAOD{
    AuxillaryMeasurementAuxContainer3D_v1::AuxillaryMeasurementAuxContainer3D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 3);
        AUX_MEASUREMENTVAR(localPosition, 3);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
