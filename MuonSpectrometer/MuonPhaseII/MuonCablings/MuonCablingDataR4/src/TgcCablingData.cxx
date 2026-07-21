#include "MuonCablingDataR4/TgcCablingData.h"

#include <format>
#include <ostream>

namespace MuonR4 {

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingOfflineID& obj) {
    ostr << std::format("stationName: {:2d} ", obj.stationName)
         << std::format("stationEta: {:2d} ", obj.stationEta)
         << std::format("stationPhi: {:2d} ", obj.stationPhi)
         << std::format("gasGap: {:1d} ", obj.gasGap)
         << std::format("isStrip: {:1d} ", obj.isStrip)
         << std::format("channel: {:2d} ", obj.channel);

    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingReadoutID& obj) {
    ostr << std::format("SLID: {:3d} ", obj.SLID)
         << std::format("cellAddress1: {:3d} ", obj.cellAddress1)
         << std::format("cellAddress2: {:3d} ", obj.cellAddress2)
         << std::format("hitBitmap1: {:6d} ", obj.hitBitmap1)
         << std::format("hitBitmap2: {:6d} ", obj.hitBitmap2);

    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingData& obj) {
    ostr << static_cast<const TgcCablingOfflineID&>(obj);
    ostr << " --- ";
    ostr << static_cast<const TgcCablingReadoutID&>(obj);

    return ostr;
}

}  // namespace Muon
