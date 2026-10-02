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

// EDM include(s).
#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {
  struct GCCXML_DUMMY_INSTANTIATION_XAODTRIGL1MUON {

    // Run 4

    // Sector logic data to MUCTPI object
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, SectorLogicCandDataContainer_v1 );
  };
}

#endif // XAODTRIGL1MUON_XAODTRIGL1MUONEVENTDICT_H
