/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/MdtCablingData.h"
#include <ostream>
#include <iomanip>
std::ostream& operator<<(std::ostream& ostr, const MdtCablingOffData& obj) {
    auto fill = ostr.fill();
    ostr << "stationIndex: " << std::setw(2)<<std::setfill('0')
         << static_cast<int>(obj.stationIndex) << ", ";
    ostr << "eta: " << std::setw(2) << static_cast<int>(obj.eta) << ", ";
    ostr << "phi: " << static_cast<int>(obj.phi) << ", ";
    ostr << "multilayer: " << static_cast<int>(obj.multilayer)<<std::setfill(fill);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const MdtCablingOnData& obj) {
    ostr << " subdetId: " << static_cast<int>(obj.subdetectorId) << ", ";
    ostr << " mrod: " << static_cast<int>(obj.mrod) << ", ";
    ostr << " csm: " << static_cast<int>(obj.csm);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const MdtCablingData& obj) {
    ostr << static_cast<const MdtCablingOffData&>(obj) << ", ";
    ostr << "layer: " << static_cast<int>(obj.layer) << ", ";
    ostr << "tube: " << static_cast<int>(obj.tube) << "  ---- ";
    ostr << static_cast<const MdtCablingOnData&>(obj) << ", ";
    ostr << "tdc: " << static_cast<int>(obj.tdcId) << ", ";
    ostr << "tdcChannel: " << static_cast<int>(obj.channelId);
    return ostr;
}
