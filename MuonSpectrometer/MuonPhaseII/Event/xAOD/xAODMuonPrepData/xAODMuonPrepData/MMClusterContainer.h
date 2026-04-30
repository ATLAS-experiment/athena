/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MMClusterCONTAINER_H
#define XAODMUONPREPDATA_MMClusterCONTAINER_H

#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"

namespace xAOD{
   using MMClusterContainer_v1 = DataVector<MMCluster_v1>;
   using MMClusterContainer = MMClusterContainer_v1;
}
// Set up a CLID for the class:

CLASS_DEF(xAOD::MMClusterContainer, 1171576513, 1)

#endif