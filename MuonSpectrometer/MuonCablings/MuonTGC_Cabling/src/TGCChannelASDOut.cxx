/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelASDOut.h"

#include <iostream>
namespace MuonTGC_Cabling {

// Constructor
TGCChannelASDOut::TGCChannelASDOut(TGCId::SideType vside,
                                   TGCId::SignalType vsignal,
                                   TGCId::RegionType vregion, int vsector,
                                   int vlayer, int vchamber, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDOut) {
    setSideType(vside);
    setSignalType(vsignal);
    setRegionType(vregion);
    setSector(vsector);
    setLayer(vlayer);
    setChamber(vchamber);
    setChannel(vchannel);
}

TGCChannelASDOut::TGCChannelASDOut(TGCId::SideType vside,
                                   TGCId::SignalType vsignal, int voctant,
                                   int vsectorModule, int vlayer, int vchamber,
                                   int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDOut) {
    setSideType(vside);
    setSignalType(vsignal);
    setOctant(voctant);
    setSectorModule(vsectorModule);  // after setOctant() method
    setLayer(vlayer);
    setChamber(vchamber);
    setChannel(vchannel);
}

bool TGCChannelASDOut::isValid(void) const {
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
