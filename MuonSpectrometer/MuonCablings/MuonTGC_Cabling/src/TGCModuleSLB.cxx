/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

TGCModuleSLB::TGCModuleSLB(TGCId::SideType vside, TGCId::ModuleType vmodule,
                           TGCId::RegionType vregion, int vsector, int vid,
                           int vsbLoc, int vslbAddr)
  : TGCModuleId(TGCModuleId::SLB) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    identifyStationType(vmodule);
    setSector(vsector);
    setId(vid);
    m_sbLoc = vsbLoc;
    m_slbAddr = vslbAddr;
}

bool TGCModuleSLB::isValid() const {
    if ((getSideType() < TGCId::SideType::MaxSideType) &&
        (getModuleType() <= TGCId::ModuleType::SL_SLB) &&  // "=" needs to add SL SLB
        (getRegionType() < TGCId::RegionType::MaxRegionType) &&
        (getOctant() >= 0) && (getOctant() < 8) && (getId() >= 0)) {
        return true;
    }
    return false;
}

void TGCModuleSLB::identifyStationType(TGCId::ModuleType module) {
    if (module == TGCId::ModuleType::WT || module == TGCId::ModuleType::ST) {
        setStation(TGCId::StationType::M1);
    } else if (module == TGCId::ModuleType::WD || module == TGCId::ModuleType::SD) {
        setStation(TGCId::StationType::M3);
    } else if (module == TGCId::ModuleType::WI || module == TGCId::ModuleType::SI) {
        setStation(TGCId::StationType::M4);
    } else if (module == TGCId::ModuleType::SL_SLB) {
        setStation(TGCId::StationType::Undefined);
    }
}

}  // namespace MuonTGC_Cabling
