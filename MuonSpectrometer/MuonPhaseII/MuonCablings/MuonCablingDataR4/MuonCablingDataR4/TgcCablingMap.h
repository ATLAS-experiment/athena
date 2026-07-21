#ifndef MUONCABLINGDATA_TGCCABLINGMAP_H
#define MUONCABLINGDATA_TGCCABLINGMAP_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include "MuonCablingDataR4/TgcCablingData.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

class TgcIdHelper;

namespace Muon{
class IMuonIdHelperSvc;
}

namespace MuonR4{
class TgcCablingMap {
public:
    struct JsonEntry {
    std::string stationName{};
        int16_t stationNameIndex{0};

        int16_t stationEta{0};
        int16_t stationPhi{0};
        int16_t gasGap{0};
        int16_t isStrip{0};
        int16_t ASDstartChannel{0};
        int16_t channelRangeStart{0};
        int16_t channelRangeEnd{0};
        bool reversed{false};
        int16_t SLID{0};
        int16_t cellAddress1{-1};
        int16_t cellAddress2{-1};
        bool hasSecondCellAddress{false};
    };

    struct OfflineToReadoutAssociation {
        TgcCablingReadoutID readoutID{};

        int16_t ASDstartChannel{0};
        int16_t channelRangeStart{0};
        int16_t channelRangeEnd{0};
        bool reversed{false};
    };

    struct ReadoutToOfflineAssociation {
        TgcCablingOfflineID offlineID{};

        int16_t ASDstartChannel{0};
        int16_t channelRangeStart{0};
        int16_t channelRangeEnd{0};
        bool reversed{false};
    };

    using OfflToReadoutMap =
        std::multimap<TgcCablingOfflineID, OfflineToReadoutAssociation>;

    using ReadoutToOfflMap =
        std::multimap<TgcCablingReadoutID, ReadoutToOfflineAssociation>;

    TgcCablingMap(const Muon::IMuonIdHelperSvc* idHelperSvc);
    ~TgcCablingMap();

    bool getOfflineId(TgcCablingData& cablingData, MsgStream& log) const;
    bool getReadoutId(TgcCablingData& cablingData, MsgStream& log) const;

    bool convert(const TgcCablingData& cablingData,
             Identifier& id,
             bool checkValid = true) const;

    bool convert(const Identifier& id,
                 TgcCablingData& cablingData) const;

    bool insertChannels(const JsonEntry& entry, MsgStream& log);

    bool finalize(MsgStream& log);

private:
    const TgcIdHelper& m_tgcIdHelper;

    OfflToReadoutMap m_offToReadout{};
    ReadoutToOfflMap m_readoutToOff{};
};

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingMap::JsonEntry& obj);

}  
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CLASS_DEF(MuonR4::TgcCablingMap, 52396898, 1);
CONDCONT_DEF(MuonR4::TgcCablingMap, 150802588);
#endif
