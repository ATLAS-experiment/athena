/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL0MUON_XAODTRIGL0MUONEVENTDICT_H
#define XAODTRIGL0MUON_XAODTRIGL0MUONEVENTDICT_H

// Local include(s):

// Run 4

#include "xAODTrigL0Muon/SectorLogicCandData.h"
#include "xAODTrigL0Muon/SectorLogicCandDataContainer.h"
#include "xAODTrigL0Muon/SectorLogicCandDataAuxContainer.h"
#include "xAODTrigL0Muon/versions/SectorLogicCandData_v1.h"
#include "xAODTrigL0Muon/versions/SectorLogicCandDataContainer_v1.h"
#include "xAODTrigL0Muon/versions/SectorLogicCandDataAuxContainer_v1.h"

// EDM include(s).
#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {
  struct GCCXML_DUMMY_INSTANTIATION_XAODTRIGL0MUON {

    // Run 4

    // Sector logic data to MUCTPI object
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, SectorLogicCandDataContainer_v1 );
  };
}

#endif // XAODTRIGL0MUON_XAODTRIGL0MUONEVENTDICT_H
