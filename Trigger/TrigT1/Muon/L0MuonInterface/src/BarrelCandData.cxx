/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "L0MuonInterface/BarrelCandData.h"

namespace L0Muon {
  
BarrelCandData::BarrelCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
: ICandData(subdetectorId, sectorId, bcTag) {} 

}

