/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelHPBIn.h"

#include "MuonTGC_Cabling/TGCModuleHPB.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelHPBIn::TGCChannelHPBIn(TGCId::SideType vside,
                                 TGCId::SignalType vsignal,
                                 TGCId::RegionType vregion, int vsector,
                                 int vid, int vblock, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::HPBIn) {
    setSideType(vside);
    setSignalType(vsignal);
    setRegionType(vregion);
    setSector(vsector);
    setId(vid);
    setBlock(vblock);
    setChannel(vchannel);
}

TGCModuleId* TGCChannelHPBIn::getModule(void) const {
    return (new TGCModuleHPB(getSideType(), getSignalType(), getRegionType(),
                             getSector(), getId()));
}

bool TGCChannelHPBIn::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getSignalType() > TGCId::NoSignalType) &&
        (getSignalType() < TGCId::MaxSignalType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getId() >= 0) && (getBlock() >= 0) &&
        (getChannel() >= 0)) {
        return true;
    }
    return false;
}

const int TGCChannelHPBIn::s_numberOfBlock = 2;
const int TGCChannelHPBIn::s_channelInBlock = 12;
const int TGCChannelHPBIn::s_slbInBlock = 3;

int TGCChannelHPBIn::getNumberOfBlock(void) {
    return s_numberOfBlock;
}

int TGCChannelHPBIn::getChannelInBlock(void) {
    return s_channelInBlock;
}

int TGCChannelHPBIn::getSlbInBlock(void) {
    return s_slbInBlock;
}

}  // namespace MuonTGC_Cabling
