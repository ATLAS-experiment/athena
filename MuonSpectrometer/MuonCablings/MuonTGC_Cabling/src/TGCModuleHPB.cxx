/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleHPB.h"

namespace MuonTGC_Cabling {

// Constructor
TGCModuleHPB::TGCModuleHPB(TGCId::SideType side, TGCId::SignalType signal,
                           TGCId::RegionType region, int sector, int id)
    : TGCModuleId(TGCModuleId::HPB) {
    setSideType(side);
    setSignalType(signal);
    setRegionType(region);
    setSector(sector);
    setId(id);
}

bool TGCModuleHPB::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getSignalType() > TGCId::NoSignalType) &&
        (getSignalType() < TGCId::MaxSignalType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
