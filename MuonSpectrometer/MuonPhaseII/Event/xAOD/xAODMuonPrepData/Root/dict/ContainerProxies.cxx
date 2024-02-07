/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODMuonPrepData/versions/MdtDriftCircleContainer_v1.h"
#include "xAODMuonPrepData/versions/RpcStripContainer_v1.h"
#include "xAODMuonPrepData/versions/TgcStripContainer_v1.h"
#include "xAODMuonPrepData/versions/MMClusterContainer_v1.h"
#include "xAODMuonPrepData/versions/sTgcStripContainer_v1.h"
#include "xAODMuonPrepData/versions/sTgcWireContainer_v1.h"
#include "xAODMuonPrepData/versions/sTgcPadContainer_v1.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY(xAOD, MdtDriftCircleContainer_v1);
ADD_NS_DV_PROXY(xAOD, RpcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, TgcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, MMClusterContainer_v1);
ADD_NS_DV_PROXY(xAOD, sTgcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, sTgcWireContainer_v1);
ADD_NS_DV_PROXY(xAOD, sTgcPadContainer_v1);
