#ifndef MUONCABLINGDATA_TGCCABLINGMAP_H
#define MUONCABLINGDATA_TGCCABLINGMAP_H

#include <cstdint>
#include <map>
#include <string>

#include "MuonCablingDataR4/TgcCablingData.h"
#include "Identifier/Identifier.h"

#include <iosfwd>

class TgcIdHelper;

namespace Muon{
class IMuonIdHelperSvc;
}

namespace MuonR4{
class TgcCablingMap {
public:
    struct JsonEntry : public TgcCablingData {
    std::string stationNameString{};
    int16_t ASDstartChannel{0};
    int16_t channelRangeStart{0};
    int16_t channelRangeEnd{0};
    int16_t offlineChannelStart{0};
    bool reversed{false};
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
CLASS_DEF(MuonR4::TgcCablingMap, 200454606, 1);
CONDCONT_DEF(MuonR4::TgcCablingMap, 176178564);
#endif
