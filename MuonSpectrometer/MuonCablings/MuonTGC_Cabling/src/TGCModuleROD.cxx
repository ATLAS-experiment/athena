/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleROD.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleROD::TGCModuleROD(TGCId::SideType vside, int vreadoutSector)
    : TGCModuleId(TGCModuleId::ROD) {
    setSideType(vside);
    setReadoutSector(vreadoutSector);
    int rodId = vreadoutSector + 1;
    setId(rodId);
}

bool TGCModuleROD::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) && (getReadoutSector() >= 0) &&
        (getReadoutSector() < N_RODS)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
