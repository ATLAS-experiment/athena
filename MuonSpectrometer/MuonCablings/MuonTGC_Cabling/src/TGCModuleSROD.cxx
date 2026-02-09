/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleSROD.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleSROD::TGCModuleSROD(TGCId::SideType vside, int vreadoutSector)
    : TGCModuleId(TGCModuleId::SROD) {
    setSideType(vside);
    setReadoutSector(vreadoutSector);
    int srodId = vreadoutSector + 1;
    setId(srodId);
}

bool TGCModuleSROD::isValid(void) const {
    return (getSideType() > TGCId::NoSideType) &&
           (getSideType() < TGCId::MaxSideType) && (getReadoutSector() >= 0) &&
           (getReadoutSector() < N_RODS);
}

}  // namespace MuonTGC_Cabling
