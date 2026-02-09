/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModulePP.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModulePP::TGCModulePP(TGCId::SideType vside, TGCId::ModuleType vmodule,
                         TGCId::RegionType vregion, int vsector, int vid)
    : TGCModuleId(TGCModuleId::PP) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
}

bool TGCModulePP::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getModuleType() > TGCId::NoModuleType) &&
        (getModuleType() < TGCId::MaxModuleType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
