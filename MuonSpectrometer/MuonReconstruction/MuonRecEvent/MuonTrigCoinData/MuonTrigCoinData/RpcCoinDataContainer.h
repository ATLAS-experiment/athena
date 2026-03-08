/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRIGCOINDATA_RPCCOINDATACONTAINER_H
#define MUONTRIGCOINDATA_RPCCOINDATACONTAINER_H

#include "MuonTrigCoinData/MuonCoinDataContainer.h"
#include "MuonTrigCoinData/RpcCoinDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
    
using RpcCoinDataContainer =  MuonCoinDataContainer< RpcCoinDataCollection >;

}

CLASS_DEF( Muon::RpcCoinDataContainer , 1190485326 , 1 )

#endif
