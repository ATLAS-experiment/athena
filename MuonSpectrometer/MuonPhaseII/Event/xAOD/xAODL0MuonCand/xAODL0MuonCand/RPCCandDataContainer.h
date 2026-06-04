/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef xAODL0MUONCAND_RPCCANDDATACONTAINER_H
#define xAODL0MUONCAND_RPCCANDDATACONTAINER_H

#include "xAODL0MuonCand/RPCCandData.h"
#include "xAODL0MuonCand/versions/RPCCandDataContainer_v1.h"


namespace xAOD {
    typedef RPCCandDataContainer_v1 RPCCandDataContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RPCCandDataContainer , 1332285776 , 1 )

#endif // xAODL0MUONCAND_RPCCANDDATACONTAINER_H



