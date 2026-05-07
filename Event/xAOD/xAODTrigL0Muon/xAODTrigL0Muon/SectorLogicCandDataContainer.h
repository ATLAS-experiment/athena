/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL0MUON_SECTORLOGICCANDDATACONTAINER_H
#define XAODTRIGL0MUON_SECTORLOGICCANDDATACONTAINER_H

// Local include(s):
#include "xAODTrigL0Muon/SectorLogicCandData.h"
#include "xAODTrigL0Muon/versions/SectorLogicCandDataContainer_v1.h"

namespace xAOD{
   typedef SectorLogicCandDataContainer_v1 SectorLogicCandDataContainer;
}

// Set up a CLID for the container:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::SectorLogicCandDataContainer, 1076817997, 1 )

#endif // XAODTRIGL0MUON_SECTORLOGICCANDDATACONTAINER_H
