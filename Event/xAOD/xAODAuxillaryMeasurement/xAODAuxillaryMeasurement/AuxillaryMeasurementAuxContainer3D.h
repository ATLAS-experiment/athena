/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTAUXCONTAINER3D_H
#define XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTAUXCONTAINER3D_H

#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurementAuxContainer3D_v1.h"
namespace xAOD{
    using AuxillaryMeasurementAuxContainer3D = AuxillaryMeasurementAuxContainer3D_v1;
}

// Set up a CLID for the container:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::AuxillaryMeasurementAuxContainer3D , 1084769315 , 1 );
#endif