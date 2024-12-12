/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/RpcCablingData.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"
#include <format>

namespace Muon{
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingOfflineID& obj) {
        ostr <<std::format("name: {:2d} ", obj.stationIndex)   
             <<std::format(" eta: {:2d} ", obj.eta)
             <<std::format(" phi: {:2d} ", obj.phi)
             <<std::format("doubletR: {:1d} ", obj.doubletR)
             <<std::format("doubletPhi: {:1d} ", obj.doubletPhi)
             <<std::format("doubletZ: {:1d} ", obj.doubletZ)
             <<std::format("gasGap: {:1d} ", obj.gasGap)
             <<std::format("isPhi: {:1d} ", obj.measuresPhi())
             <<std::format("side: {:1d} ", obj.stripSide());
        return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingOnlineID& obj) {
        ostr << std::format("subDetector: {:3d} ",obj.subDetector);
        ostr << std::format("tdcSector: {:2d} ", obj.tdcSector);
        ostr << std::format("tdc: {:2d} ", obj.tdc);
        return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingData& obj) {
        ostr << static_cast<const RpcCablingOfflineID&>(obj);
        ostr << std::format("strip: {:2d} ", obj.strip) << " --- ";
        ostr << static_cast<const RpcCablingOnlineID&>(obj);
        ostr << std::format("channelId: {:2d}", obj.channelId);
        return ostr;
    }
}
