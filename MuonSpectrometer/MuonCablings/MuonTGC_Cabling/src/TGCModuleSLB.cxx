/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleSLB::TGCModuleSLB(TGCId::SideType vside, TGCId::ModuleType vmodule,
                           TGCId::RegionType vregion, int vsector, int vid,
                           int vsbLoc, int vslbAddr)
    : TGCModuleId(TGCModuleId::SLB) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    m_sbLoc = vsbLoc;
    m_slbAddr = vslbAddr;
}

bool TGCModuleSLB::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getModuleType() > TGCId::NoModuleType) &&
        (getModuleType() < TGCId::MaxModuleType + 1) &&  // add SLB SL
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
