/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_VERSIONS_SECTORLOGICCANDDATACONTAINER_V1_H
#define XAODTRIGL1MUON_VERSIONS_SECTORLOGICCANDDATACONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"

// Local include(s):
#include "xAODTrigL1Muon/versions/SectorLogicCandData_v1.h"

namespace xAOD{
   /// Declare the SL data container type
   typedef DataVector< xAOD::SectorLogicCandData_v1 > SectorLogicCandDataContainer_v1;
}

#endif // XAODTRIGL1MUON_VERSIONS_SECTORLOGICCANDDATACONTAINER_V1_H
