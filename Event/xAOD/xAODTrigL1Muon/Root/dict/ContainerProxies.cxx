/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODTrigL1Muon/versions/SectorLogicCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1RPCCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1TGCCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1MDTCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1NSWCandDataContainer_v1.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY( xAOD, SectorLogicCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, L1RPCCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, L1TGCCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, L1MDTCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, L1NSWCandDataContainer_v1 );
