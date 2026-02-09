/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelASDIn.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelASDIn::TGCChannelASDIn(TGCId::SideType vside,
                                 TGCId::SignalType vsignal,
                                 TGCId::RegionType vregion, int vsector,
                                 int vlayer, int vchamber, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDIn) {
    setSideType(vside);
    setSignalType(vsignal);
    setRegionType(vregion);
    setLayer(vlayer);
    TGCChannelASDIn::setSector(vsector);
    setChamber(vchamber);
    setChannel(vchannel);
}

void TGCChannelASDIn::setSector(int sector) {
    if (isEndcap() && !isInner()) {
        TGCId::setSector((sector + 1) % TGCId::NUM_ENDCAP_SECTOR);
    } else {
        TGCId::setSector(sector % TGCId::NUM_FORWARD_SECTOR);
    }
}

int TGCChannelASDIn::getSector(void) const {
    int sector;
    if (isEndcap() && !isInner()) {
        sector = TGCId::getSector() - 1;
        if (sector <= 0) {
            sector += TGCId::NUM_ENDCAP_SECTOR;
        }
    } else {
        sector = TGCId::getSector();
        if (sector <= 0) {
            sector += TGCId::NUM_FORWARD_SECTOR;
        }
    }

    return sector;
}

bool TGCChannelASDIn::isValid(void) const {
    if ((getSideType() > TGCId::NoSideType) &&
        (getSideType() < TGCId::MaxSideType) &&
        (getSignalType() > TGCId::NoSignalType) &&
        (getSignalType() < TGCId::MaxSignalType) &&
        (getRegionType() > TGCId::NoRegionType) &&
        (getRegionType() < TGCId::MaxRegionType) && (getOctant() >= 0) &&
        (getOctant() < 8) && (getLayer() >= 0) && (getChamber() >= 0) &&
        (getChannel() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
