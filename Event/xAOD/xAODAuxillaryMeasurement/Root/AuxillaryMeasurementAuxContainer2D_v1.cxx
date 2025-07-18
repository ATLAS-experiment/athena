/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurementAuxContainer2D_v1.h"
namespace xAOD{
    AuxillaryMeasurementAuxContainer2D_v1::AuxillaryMeasurementAuxContainer2D_v1():
        AuxContainerBase{}{
        AUX_MEASUREMENTVAR(localCovariance, 2);
        AUX_MEASUREMENTVAR(localPosition, 2);
        AUX_VARIABLE(calibProjector);
        AUX_VARIABLE(surfaceLink);
    }
}
