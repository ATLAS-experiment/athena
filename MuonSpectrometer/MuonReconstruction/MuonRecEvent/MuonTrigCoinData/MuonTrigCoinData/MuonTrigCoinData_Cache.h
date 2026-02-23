/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "EventContainers/IdentifiableCache.h"
#include "MuonTrigCoinData/RpcCoinDataCollection.h"
#include "MuonTrigCoinData/TgcCoinDataCollection.h"

// Typedef the different templates which are likely to be used later on
using RpcCoinDataCollection_Cache =
    EventContainers::IdentifiableCache<Muon::RpcCoinDataCollection>;
using TgcCoinDataCollection_Cache =
    EventContainers::IdentifiableCache<Muon::TgcCoinDataCollection>;

CLASS_DEF(RpcCoinDataCollection_Cache, 1162490086, 1)
CLASS_DEF(TgcCoinDataCollection_Cache, 1080713821, 1)
