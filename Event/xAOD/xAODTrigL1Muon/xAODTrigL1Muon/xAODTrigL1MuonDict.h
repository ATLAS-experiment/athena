/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1MUON_XAODTRIGL1MUONEVENTDICT_H
#define XAODTRIGL1MUON_XAODTRIGL1MUONEVENTDICT_H

// Local include(s):

// Run 4

#include "xAODTrigL1Muon/SectorLogicCandData.h"
#include "xAODTrigL1Muon/SectorLogicCandDataContainer.h"
#include "xAODTrigL1Muon/SectorLogicCandDataAuxContainer.h"
#include "xAODTrigL1Muon/versions/SectorLogicCandData_v1.h"
#include "xAODTrigL1Muon/versions/SectorLogicCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/SectorLogicCandDataAuxContainer_v1.h"

#include "xAODTrigL1Muon/IL1CandData.h"
#include "xAODTrigL1Muon/versions/IL1CandData_v1.h"

#include "xAODTrigL1Muon/L1RPCCandData.h"
#include "xAODTrigL1Muon/L1RPCCandDataContainer.h"
#include "xAODTrigL1Muon/L1RPCCandDataAuxContainer.h"
#include "xAODTrigL1Muon/versions/L1RPCCandData_v1.h"
#include "xAODTrigL1Muon/versions/L1RPCCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1RPCCandDataAuxContainer_v1.h"

#include "xAODTrigL1Muon/L1TGCCandData.h"
#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"
#include "xAODTrigL1Muon/L1TGCCandDataAuxContainer.h"
#include "xAODTrigL1Muon/versions/L1TGCCandData_v1.h"
#include "xAODTrigL1Muon/versions/L1TGCCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1TGCCandDataAuxContainer_v1.h"

#include "xAODTrigL1Muon/L1MDTCandData.h"
#include "xAODTrigL1Muon/L1MDTCandDataContainer.h"
#include "xAODTrigL1Muon/L1MDTCandDataAuxContainer.h"
#include "xAODTrigL1Muon/versions/L1MDTCandData_v1.h"
#include "xAODTrigL1Muon/versions/L1MDTCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1MDTCandDataAuxContainer_v1.h"

#include "xAODTrigL1Muon/L1NSWCandData.h"
#include "xAODTrigL1Muon/L1NSWCandDataContainer.h"
#include "xAODTrigL1Muon/L1NSWCandDataAuxContainer.h"
#include "xAODTrigL1Muon/versions/L1NSWCandData_v1.h"
#include "xAODTrigL1Muon/versions/L1NSWCandDataContainer_v1.h"
#include "xAODTrigL1Muon/versions/L1NSWCandDataAuxContainer_v1.h"

// EDM include(s).
#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {
  struct GCCXML_DUMMY_INSTANTIATION_XAODTRIGL1MUON {

    // Run 4

    // Sector logic data to MUCTPI object
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, SectorLogicCandDataContainer_v1 );

    // RPC/TGC/MDT/NSW candidate data
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, L1RPCCandDataContainer_v1 );
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, L1TGCCandDataContainer_v1 );
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, L1MDTCandDataContainer_v1 );
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, L1NSWCandDataContainer_v1 );
  };
}

#endif // XAODTRIGL1MUON_XAODTRIGL1MUONEVENTDICT_H
