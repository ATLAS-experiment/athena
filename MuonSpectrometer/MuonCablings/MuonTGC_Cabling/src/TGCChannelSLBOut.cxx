/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelSLBOut.h"

#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelSLBOut::TGCChannelSLBOut(TGCId::SideType vside,
                                   TGCId::ModuleType vmodule,
                                   TGCId::RegionType vregion, int vsector,
                                   int vid, int vblock, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::SLBOut) {
    setSideType(vside);
    setModuleType(vmodule);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    setBlock(vblock);
    setChannel(vchannel);
}

std::unique_ptr<TGCModuleId> TGCChannelSLBOut::getModule() const {
    return std::make_unique<TGCModuleSLB>(
        getSideType(), getModuleType(), getRegionType(), getSector(), getId());
}

bool TGCChannelSLBOut::isValid() const {
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

const int TGCChannelSLBOut::s_numberOfBlockInWD = 2;
const int TGCChannelSLBOut::s_numberOfBlockInSD = 2;
const int TGCChannelSLBOut::s_numberOfBlockInWT = 3;
const int TGCChannelSLBOut::s_numberOfBlockInST = 8;
const int TGCChannelSLBOut::s_numberOfLayerInWD = 2;
const int TGCChannelSLBOut::s_numberOfLayerInSD = 2;
const int TGCChannelSLBOut::s_numberOfLayerInWT = 3;
const int TGCChannelSLBOut::s_numberOfLayerInST = 2;
const int TGCChannelSLBOut::s_channelInBlockForWD = 32;
const int TGCChannelSLBOut::s_channelInBlockForSD = 32;
const int TGCChannelSLBOut::s_channelInBlockForWT = 32;
const int TGCChannelSLBOut::s_channelInBlockForST = 16;

int TGCChannelSLBOut::getNumberOfBlock(TGCId::ModuleType moduleType) {
    switch (moduleType) {
        case TGCId::ModuleType::WD:
            return s_numberOfBlockInWD;
        case TGCId::ModuleType::SD:
            return s_numberOfBlockInSD;
        case TGCId::ModuleType::WT:
            return s_numberOfBlockInWT;
        case TGCId::ModuleType::ST:
            return s_numberOfBlockInST;
        default:
            break;
    }
    return -1;
}

int TGCChannelSLBOut::getNumberOfLayer(TGCId::ModuleType moduleType) {
    switch (moduleType) {
        case TGCId::ModuleType::WD:
            return s_numberOfLayerInWD;
        case TGCId::ModuleType::SD:
            return s_numberOfLayerInSD;
        case TGCId::ModuleType::WT:
            return s_numberOfLayerInWT;
        case TGCId::ModuleType::ST:
            return s_numberOfLayerInST;
        default:
            break;
    }
    return -1;
}

int TGCChannelSLBOut::getChannelInBlock(TGCId::ModuleType moduleType) {
    switch (moduleType) {
        case TGCId::ModuleType::WD:
            return s_channelInBlockForWD;
        case TGCId::ModuleType::SD:
            return s_channelInBlockForSD;
        case TGCId::ModuleType::WT:
            return s_channelInBlockForWT;
        case TGCId::ModuleType::ST:
            return s_channelInBlockForST;
        default:
            break;
    }
    return -1;
}

}  // namespace MuonTGC_Cabling
