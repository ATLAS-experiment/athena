/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MuonInterface_RPCCANDDATACONTAINER_H
#define L1MuonInterface_RPCCANDDATACONTAINER_H

#include "L1MuonInterface/RPCCandData.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace L1Muon {

using RPCCandDataContainer = DataVector<RPCCandData>;

}  // namespace L1Muon

CLASS_DEF( L1Muon::RPCCandDataContainer , 1321396049 , 1 )

#endif  // L1MuonInterface_RPCCANDDATACONTAINER_H
