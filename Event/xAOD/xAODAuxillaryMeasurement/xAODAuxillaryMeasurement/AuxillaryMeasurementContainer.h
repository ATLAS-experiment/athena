/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTCONTAINER_H
#define XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTCONTAINER_H

#include "AthContainers/DataVector.h"

#include "xAODAuxillaryMeasurement/AuxillaryMeasurement.h"
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurementContainer_v1.h"
namespace xAOD{
    using AuxillaryMeasurementContainer = AuxillaryMeasurementContainer_v1;
}

// Set up a CLID for the container:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::AuxillaryMeasurementContainer , 1160886801 , 1 );
#endif