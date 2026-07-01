/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MDTDRIFTCIRCLECONTAINER_H
#define XAODMUONPREPDATA_MDTDRIFTCIRCLECONTAINER_H

#include "xAODMuonPrepData/MuonMeasurementContainer.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"

namespace xAOD{
   using MdtDriftCircleContainer_v1 = DataVector<MdtDriftCircle_v1>;
   using MdtDriftCircleContainer = MdtDriftCircleContainer_v1;
}

// Set up a CLID for the class:
CLASS_DEF(xAOD::MdtDriftCircleContainer, 1098001914, 1)

#endif