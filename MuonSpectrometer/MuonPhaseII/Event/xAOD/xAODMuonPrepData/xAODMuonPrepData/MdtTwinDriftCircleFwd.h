/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MdtTwinDriftCircleFWD_H
#define XAODMUONPREPDATA_MdtTwinDriftCircleFWD_H
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/MdtDriftCircleFwd.h"
/** @brief Forward declaration of the xAOD::MdtTwinDriftCircle */
namespace xAOD{
   class MdtDriftCircle_v1;
   class MdtTwinDriftCircle_v1;
   using MdtTwinDriftCircle = MdtTwinDriftCircle_v1;

   class MdtTwinDriftCircleAuxContainer_v1;
   using MdtTwinDriftCircleAuxContainer = MdtTwinDriftCircleAuxContainer_v1;
}

DATAVECTOR_BASE(xAOD::MdtTwinDriftCircle_v1, xAOD::MdtDriftCircle_v1);

namespace xAOD{
   using MdtTwinDriftCircleContainer_v1 = DataVector<MdtTwinDriftCircle_v1>;
   using MdtTwinDriftCircleContainer = MdtTwinDriftCircleContainer_v1;
}
#endif
