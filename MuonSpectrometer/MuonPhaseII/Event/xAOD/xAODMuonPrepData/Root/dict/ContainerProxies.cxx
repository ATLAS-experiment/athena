/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStrip2DContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"

#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "xAODMuonPrepData/sTgcStripContainer.h"
#include "xAODMuonPrepData/sTgcWireContainer.h"
#include "xAODMuonPrepData/sTgcPadContainer.h"

#include "xAODMuonPrepData/CombinedMuonStripContainer.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY(xAOD, MdtDriftCircleContainer_v1);
ADD_NS_DV_PROXY(xAOD, MdtTwinDriftCircleContainer_v1);

ADD_NS_DV_PROXY(xAOD, RpcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, RpcStrip2DContainer_v1);
ADD_NS_DV_PROXY(xAOD, TgcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, MMClusterContainer_v1);

ADD_NS_DV_PROXY(xAOD, sTgcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, sTgcWireContainer_v1);
ADD_NS_DV_PROXY(xAOD, sTgcPadContainer_v1);

ADD_NS_DV_PROXY(xAOD, CombinedMuonStripContainer);
