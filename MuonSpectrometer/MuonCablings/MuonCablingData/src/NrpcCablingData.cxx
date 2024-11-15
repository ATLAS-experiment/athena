/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/NrpcCablingData.h"

std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOfflineID& obj) {
    ostr << "StationIndex: " << static_cast<int>(obj.stationIndex)
         << " eta: " << static_cast<int>(obj.eta)
         << " phi: " << static_cast<int>(obj.phi)
         << " doubletR: " << static_cast<int>(obj.doubletR)
         << " doubletPhi: " << static_cast<int>(obj.doubletPhi)
         << " doubletZ: " << static_cast<int>(obj.doubletZ)
         << " gasGap: " << static_cast<int>(obj.gasGap)
         << " measPhi: " << static_cast<int>(obj.measuresPhi())
         << " stripSide: "<<static_cast<int>(obj.stripSide());
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOnlineID& obj) {
    ostr << " subDetector: " << static_cast<int>(obj.subDetector);
    ostr << " tdcSector: " << static_cast<int>(obj.tdcSector);
    ostr << " tdc: " << static_cast<int>(obj.tdc);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingData& obj) {
    ostr << static_cast<const NrpcCablingOfflineID&>(obj);
    ostr << " strip:  " << static_cast<int>(obj.strip) << " --- ";
    ostr << static_cast<const NrpcCablingOnlineID&>(obj);
    ostr << " channelId: " << static_cast<int>(obj.channelId);
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const NrpcTdcStripRange& obj){
     ostr<<" firstStrip: "<<static_cast<int>(obj.firstStrip)
         <<" lastStrip: "<<static_cast<int>(obj.lastStrip);
     return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcTdcChannelRange& obj){
    ostr<<" first channel: "<<static_cast<int>(obj.firstChannel)
        <<" last channel: "<<static_cast<int>(obj.lastChannel);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingCoolData& obj){
    ostr << static_cast<const NrpcCablingOfflineID&>(obj);
    ostr << static_cast<const NrpcTdcStripRange&>(obj)<< " --- ";
    ostr << static_cast<const NrpcCablingOnlineID&>(obj);
    ostr <<  static_cast<const NrpcTdcChannelRange&>(obj);
    return ostr; 
   
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablOnDataByTdc& obj){
    ostr << static_cast<const NrpcCablingOnlineID&>(obj);
    ostr << static_cast<const NrpcTdcStripRange&>(obj);
    ostr <<  static_cast<const NrpcTdcChannelRange&>(obj);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablOnDataByStrip& obj){
    ostr << static_cast<const NrpcCablingOnlineID&>(obj);
    ostr << static_cast<const NrpcTdcStripRange&>(obj);
    ostr <<  static_cast<const NrpcTdcChannelRange&>(obj);
    return ostr;
}

