/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_sTgcWireHitFWD_H
#define XAODMUONPREPDATA_sTgcWireHitFWD_H
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/sTgcMeasurementFwd.h"
/** @brief Forward declaration of the xAOD::sTgcWireHit */
namespace xAOD{
   class sTgcMeasurement_v1;
   class sTgcWireHit_v1;
   using sTgcWireHit = sTgcWireHit_v1;

   class sTgcWireAuxContainer_v1;
   using sTgcWireAuxContainer = sTgcWireAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::sTgcWireHit_v1, xAOD::sTgcMeasurement_v1);

namespace xAOD{
   using sTgcWireContainer_v1 = DataVector<sTgcWireHit_v1>;
   using sTgcWireContainer = sTgcWireContainer_v1;
}
#endif
