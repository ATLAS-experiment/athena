/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_L1RPCCANDDATACONTAINER_H
#define XAODTRIGL1MUON_L1RPCCANDDATACONTAINER_H

#include "xAODTrigL1Muon/L1RPCCandData.h"
#include "xAODTrigL1Muon/versions/L1RPCCandDataContainer_v1.h"


namespace xAOD {
    typedef L1RPCCandDataContainer_v1 L1RPCCandDataContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::L1RPCCandDataContainer , 1231556913 , 1 )

#endif // XAODTRIGL1MUON_L1RPCCANDDATACONTAINER_H
