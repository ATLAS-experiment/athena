/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTDRIFTCIRCLEFWD_H
#define XAODMUONPREPDATA_MDTDRIFTCIRCLEFWD_H
#include "AthContainers/DataVector.h"

/** @brief Forward declaration of the xAOD::MdtDriftCircle */
namespace xAOD{
   class UncalibratedMeasurement_v1;
   class MdtDriftCircle_v1;
   using MdtDriftCircle = MdtDriftCircle_v1;

   class MdtDriftCircleAuxContainer_v1;
   using MdtDriftCircleAuxContainer = MdtDriftCircleAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::MdtDriftCircle_v1, xAOD::UncalibratedMeasurement_v1);

namespace xAOD{
   using MdtDriftCircleContainer_v1 = DataVector<MdtDriftCircle_v1>;
   using MdtDriftCircleContainer = MdtDriftCircleContainer_v1;
}
#endif
