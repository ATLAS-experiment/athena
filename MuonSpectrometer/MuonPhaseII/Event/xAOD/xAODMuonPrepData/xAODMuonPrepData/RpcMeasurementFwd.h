/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RpcMeasurementFWD_H
#define XAODMUONPREPDATA_RpcMeasurementFWD_H
#include "AthContainers/DataVector.h"

/** @brief Forward declaration of the xAOD::RpcMeasurement */
namespace xAOD{
   class UncalibratedMeasurement_v1;
   class RpcMeasurement_v1;
   using RpcMeasurement = RpcMeasurement_v1;
}

DATAVECTOR_BASE(xAOD::RpcMeasurement_v1, xAOD::UncalibratedMeasurement_v1);

namespace xAOD{
   using RpcMeasurementContainer_v1 = DataVector<RpcMeasurement_v1>;
   using RpcMeasurementContainer = RpcMeasurementContainer_v1;
}
#endif
