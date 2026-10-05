/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_L1MDTCANDDATACONTAINER_H
#define XAODTRIGL1MUON_L1MDTCANDDATACONTAINER_H

#include "xAODTrigL1Muon/L1MDTCandData.h"
#include "xAODTrigL1Muon/versions/L1MDTCandDataContainer_v1.h"


namespace xAOD {
    typedef L1MDTCandDataContainer_v1 L1MDTCandDataContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::L1MDTCandDataContainer , 1140905149 , 1 )
#endif // XAODTRIGL1MUON_L1MDTCANDDATACONTAINER_H
