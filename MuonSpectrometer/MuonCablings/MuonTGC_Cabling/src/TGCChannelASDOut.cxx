/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCChannelASDOut.h"

namespace MuonTGC_Cabling {

// Constructor
TGCChannelASDOut::TGCChannelASDOut(TGCId::SideType vside,
                                   TGCId::SignalType vsignal,
                                   TGCId::RegionType vregion, int vsector,
                                   int vlayer, int vchamber, int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDOut) {
    setSideType(vside);
    setRegionType(vregion);
    setSector(vsector);
    setSignalAndLayer(vsignal, vlayer);
    setChamber(vchamber);
    setChannel(vchannel);
}

TGCChannelASDOut::TGCChannelASDOut(TGCId::SideType vside,
                                   TGCId::SignalType vsignal, int voctant,
                                   int vsectorModule, int vlayer, int vchamber,
                                   int vchannel)
    : TGCChannelId(TGCChannelId::ChannelIdType::ASDOut) {
    setSideType(vside);
    setSignalAndLayer(vsignal, vlayer); // SignalType as well as Layer (to define station,Module)
    setOctant(voctant);
    setSectorModule(vsectorModule);  // after setOctant() method
    setChamber(vchamber);
    setChannel(vchannel);
}

bool TGCChannelASDOut::isValid() const {
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
