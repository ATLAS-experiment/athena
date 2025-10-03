/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_RPCCANDDATACONTAINER_H
#define L0MuonInterface_RPCCANDDATACONTAINER_H

#include "L0MuonInterface/RPCCandData.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace L0Muon {

using RPCCandDataContainer = DataVector<RPCCandData>;

}  // namespace L0Muon

CLASS_DEF( L0Muon::RPCCandDataContainer , 1321396049 , 1 )

#endif  // L0MuonInterface_RPCCANDDATACONTAINER_H
