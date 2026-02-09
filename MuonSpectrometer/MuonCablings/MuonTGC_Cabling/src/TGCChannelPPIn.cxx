/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelPPIn.h"

#include "MuonTGC_Cabling/TGCModulePP.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelPPIn::TGCChannelPPIn(TGCId::SideType vside, TGCId::ModuleType vmodule,
                               TGCId::RegionType vregion, int vsector, int vid,
                               int vblock, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::PPIn) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    setBlock(vblock);
    setChannel(vchannel);
}

TGCModuleId* TGCChannelPPIn::getModule(void) const {
    return (new TGCModulePP(getSideType(), getModuleType(), getRegionType(),
                            getSector(), getId()));
}

bool TGCChannelPPIn::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getModuleType() > TGCId::NoModuleType) &&
        (getModuleType() < TGCId::MaxModuleType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0) && (getBlock() >= 0) &&
        (getChannel() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
