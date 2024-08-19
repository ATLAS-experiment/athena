/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_sTgcMeasurementFWD_H
#define XAODMUONPREPDATA_sTgcMeasurementFWD_H
#include "AthContainers/DataVector.h"

/** @brief Forward declaration of the xAOD::sTgcMeasurement */
namespace xAOD{
   class UncalibratedMeasurement_v1;
   class sTgcMeasurement_v1;
   using sTgcMeasurement = sTgcMeasurement_v1;

   class sTgcMeasurementAuxContainer_v1;
   using sTgcMeasurementAuxContainer = sTgcMeasurementAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::sTgcMeasurement_v1, xAOD::UncalibratedMeasurement_v1);

namespace xAOD{    
   using sTgcMeasContainer_v1 = DataVector<sTgcMeasurement_v1>;
   using sTgcMeasContainer = sTgcMeasContainer_v1;
}
#endif
