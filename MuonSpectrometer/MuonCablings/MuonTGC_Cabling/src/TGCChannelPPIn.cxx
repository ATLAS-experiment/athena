/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelPPIn.h"

#include "MuonTGC_Cabling/TGCModulePP.h"

namespace MuonTGC_Cabling {

TGCChannelPPIn::TGCChannelPPIn(TGCId::SideType vside, TGCId::StationType vstation,
                               TGCId::ModuleType vmodule, TGCId::RegionType vregion,
                               int vsector, int vid, int vblock, int vchannel)
  : TGCChannelId(TGCChannelId::ChannelIdType::PPIn) {
    setSideType(vside);
    setStation(vstation);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    setBlock(vblock);
    setChannel(vchannel);
}

std::unique_ptr<TGCModuleId> TGCChannelPPIn::getModule() const {
    return std::make_unique<TGCModulePP>(getSideType(), getModuleType(),
                                         getRegionType(), getSector(), getId());
}

bool TGCChannelPPIn::isValid() const {
    if ((getSideType() < TGCId::SideType::MaxSideType) &&
        (getModuleType() < TGCId::ModuleType::MaxModuleType) &&
        (getRegionType() < TGCId::RegionType::MaxRegionType) &&
        (getOctant() >= 0) && (getOctant() < 8) &&
        (getId() >= 0) && (getBlock() >= 0) &&
        (getChannel() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
