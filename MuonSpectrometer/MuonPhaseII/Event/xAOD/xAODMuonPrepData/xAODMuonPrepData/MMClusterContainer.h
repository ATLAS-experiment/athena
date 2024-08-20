/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MMClusterCONTAINER_H
#define XAODMUONPREPDATA_MMClusterCONTAINER_H

#include "xAODMuonPrepData/MMClusterFwd.h"
#include "xAODMuonPrepData/MMCluster.h"


namespace xAOD{
   using MMClusterContainer_v1 = DataVector<MMCluster_v1>;
   using MMClusterContainer = MMClusterContainer_v1;
}
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MMClusterContainer, 1171576513, 1)

#endif