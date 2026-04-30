/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRIGCOINDATA_TGCCOINDATACONTAINER_H
#define MUONTRIGCOINDATA_TGCCOINDATACONTAINER_H

#include "MuonTrigCoinData/MuonCoinDataContainer.h"
#include "MuonTrigCoinData/TgcCoinDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
    
using TgcCoinDataContainer = MuonCoinDataContainer< TgcCoinDataCollection >;

}

CLASS_DEF( Muon::TgcCoinDataContainer , 1190485325 , 1 )

#endif
