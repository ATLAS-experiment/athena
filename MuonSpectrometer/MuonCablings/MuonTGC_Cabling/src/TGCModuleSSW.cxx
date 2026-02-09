/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleSSW.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleSSW::TGCModuleSSW(TGCId::SideType vside, int vreadoutSector, int vid)
    : TGCModuleId(TGCModuleId::SSW) {
    setSideType(vside);
    setReadoutSector(vreadoutSector);
    setId(vid);
}

bool TGCModuleSSW::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) && (getReadoutSector() >= 0) &&
        (getReadoutSector() < N_RODS) && (getId() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
