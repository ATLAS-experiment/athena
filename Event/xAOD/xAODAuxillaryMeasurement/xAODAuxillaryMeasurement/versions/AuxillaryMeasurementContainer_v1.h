/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_VERSION_AUXILLARYMEASUREMENTCONTAINER_V1_H
#define XAODAUXILLARYMEASUREMENT_VERSION_AUXILLARYMEASUREMENTCONTAINER_V1_H

#include "AthContainers/DataVector.h"
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurement_v1.h"
namespace xAOD {
    using AuxillaryMeasurementContainer_v1 = DataVector<AuxillaryMeasurement_v1>;
}
#endif