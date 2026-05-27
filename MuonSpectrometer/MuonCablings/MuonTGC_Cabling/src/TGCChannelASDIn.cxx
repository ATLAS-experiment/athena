/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelASDIn.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelASDIn::TGCChannelASDIn(TGCId::SideType vside,
                                 TGCId::SignalType vsignal,
                                 TGCId::RegionType vregion, int vsector,
                                 int vlayer, int vchamber, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDIn) {
    setSideType(vside);         // SideType
    setRegionType(vregion);     // RegionType
    setSignalAndLayer(vsignal, vlayer);  // SignalType as well as Layer (to define station,Module)
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

int TGCChannelASDIn::getSector() const {
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

bool TGCChannelASDIn::isValid() const {
    if ((getSideType() < TGCId::SideType::MaxSideType) &&
        (getSignalType() < TGCId::SignalType::MaxSignalType) &&
        (getRegionType() < TGCId::RegionType::MaxRegionType) &&
        (getOctant() >= 0) && (getOctant() < 8) &&
        (getLayer() >= 0) && (getChamber() >= 0) &&
        (getChannel() >= 0)) {
        return true;
    }
    return false;
}

}  // namespace MuonTGC_Cabling
