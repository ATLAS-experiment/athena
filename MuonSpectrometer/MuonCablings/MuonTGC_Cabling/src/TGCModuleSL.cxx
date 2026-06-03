/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleSL.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleSL::TGCModuleSL(TGCId::SideType vside, TGCId::RegionType vregion,
                         int vsector)
    : TGCModuleId(TGCModuleId::SL) {
    setSideType(vside);
    setRegionType(vregion);
    setSector(vsector);
}

bool TGCModuleSL::isValid() const {
    if ((getSideType() < TGCId::SideType::MaxSideType) &&
        (getRegionType() < TGCId::RegionType::MaxRegionType) &&
        (getOctant() >= 0) && (getOctant() < 8)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
