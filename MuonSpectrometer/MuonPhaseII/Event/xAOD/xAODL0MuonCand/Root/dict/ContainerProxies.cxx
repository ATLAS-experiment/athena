/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):

#include "xAODL0MuonCand/versions/RPCCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/TGCCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/MDTCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/NSWCandDataContainer_v1.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY( xAOD, RPCCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, TGCCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, MDTCandDataContainer_v1 );
ADD_NS_DV_PROXY( xAOD, NSWCandDataContainer_v1 );
