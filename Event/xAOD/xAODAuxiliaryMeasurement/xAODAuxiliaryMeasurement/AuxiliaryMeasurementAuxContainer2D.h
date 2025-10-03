/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTAUXCONTAINER2D_H
#define XAODAUXILLARYMEASUREMENT_AUXILLARYMEASUREMENTAUXCONTAINER2D_H

#include "xAODAuxiliaryMeasurement/versions/AuxiliaryMeasurementAuxContainer2D_v1.h"
namespace xAOD{
    using AuxiliaryMeasurementAuxContainer2D = AuxiliaryMeasurementAuxContainer2D_v1;
}

// Set up a CLID for the container:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::AuxiliaryMeasurementAuxContainer2D , 1083769440 , 1 );
#endif