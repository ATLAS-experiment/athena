/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_sTgcPadHitFWD_H
#define XAODMUONPREPDATA_sTgcPadHitFWD_H
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/sTgcMeasurementFwd.h"
/** @brief Forward declaration of the xAOD::sTgcPadHit */
namespace xAOD{
   class sTgcMeasurement_v1;
   class sTgcPadHit_v1;
   using sTgcPadHit = sTgcPadHit_v1;

   class sTgcPadAuxContainer_v1;
   using sTgcPadAuxContainer = sTgcPadAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::sTgcPadHit_v1, xAOD::sTgcMeasurement_v1);

namespace xAOD{
   using sTgcPadContainer_v1 = DataVector<sTgcPadHit_v1>;
   using sTgcPadContainer = sTgcPadContainer_v1;
}
#endif
