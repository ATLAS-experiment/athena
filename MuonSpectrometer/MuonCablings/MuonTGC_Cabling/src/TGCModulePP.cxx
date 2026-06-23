/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

TGCModulePP::TGCModulePP(TGCId::SideType vside, TGCId::StationType vstation,
                         TGCId::ModuleType vmodule, TGCId::RegionType vregion,
                         int vsector, int vid)
    : TGCModuleId(TGCModuleId::PP) {
    setSideType(vside);
    setStation(vstation);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
}

bool TGCModulePP::isValid() const {
    if ((getSideType() < TGCId::SideType::MaxSideType) &&
        (getModuleType() < TGCId::ModuleType::MaxModuleType) &&
        (getRegionType() < TGCId::RegionType::MaxRegionType) &&
        (getOctant() >= 0) && (getOctant() < 8) && (getId() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
