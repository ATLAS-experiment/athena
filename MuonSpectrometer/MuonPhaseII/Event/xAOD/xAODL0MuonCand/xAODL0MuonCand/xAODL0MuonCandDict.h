/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODL0MUONCAND_XAODL0MUONCANDDICT_H
#define XAODL0MUONCAND_XAODL0MUONCANDDICT_H

// Local include(s):
// Run 4

#include "xAODL0MuonCand/RPCCandData.h"
#include "xAODL0MuonCand/RPCCandDataContainer.h"
#include "xAODL0MuonCand/versions/RPCCandData_v1.h"
#include "xAODL0MuonCand/versions/RPCCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/RPCCandDataAuxContainer_v1.h"

#include "xAODL0MuonCand/TGCCandData.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"
#include "xAODL0MuonCand/versions/TGCCandData_v1.h"
#include "xAODL0MuonCand/versions/TGCCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/TGCCandDataAuxContainer_v1.h"

#include "xAODL0MuonCand/MDTCandData.h"
#include "xAODL0MuonCand/MDTCandDataContainer.h"
#include "xAODL0MuonCand/versions/MDTCandData_v1.h"
#include "xAODL0MuonCand/versions/MDTCandDataContainer_v1.h"
#include "xAODL0MuonCand/versions/MDTCandDataAuxContainer_v1.h"

#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {

  struct GCCXML_DUMMY_INSTANTIATION_XAODTRIGL0MUON {
    // Run 4
    // Sector logic data to MUCTPI object
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, RPCCandDataContainer_v1 );
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, TGCCandDataContainer_v1 );
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, MDTCandDataContainer_v1 );

  };
}

#endif // XAODTRIGL0MUON_XAODTRIGL0MUONEVENTDICT_H
