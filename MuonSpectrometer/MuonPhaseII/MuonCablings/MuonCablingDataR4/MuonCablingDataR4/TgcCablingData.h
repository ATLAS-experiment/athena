#ifndef MUONCABLINGDATA_TGCCABLINGDATA_H
#define MUONCABLINGDATA_TGCCABLINGDATA_H

#include <compare>
#include <cstdint>
#include <iostream>
#include <set>

namespace MuonR4 {

struct TgcCablingOfflineID {
    int8_t stationName{0};
    int8_t stationEta{0};
    int8_t stationPhi{0};
    int8_t gasGap{0};
    int8_t isStrip{0};
    int8_t channel{0};

    bool operator==(const TgcCablingOfflineID&) const = default;
    auto operator<=>(const TgcCablingOfflineID&) const = default;

    bool operator!() const {
        return *this == TgcCablingOfflineID{};
    }
};

struct TgcCablingReadoutID {
    int16_t SLID{0};
    int16_t cellAddress1{0};
    int16_t cellAddress2{0};
    int16_t hitBitmap1{0};
    int16_t hitBitmap2{0};

    bool operator==(const TgcCablingReadoutID&) const = default;
    auto operator<=>(const TgcCablingReadoutID&) const = default;

    bool operator!() const {
        return *this == TgcCablingReadoutID{};
    }
};

struct TgcCablingData : public TgcCablingOfflineID,
                        public TgcCablingReadoutID {
    TgcCablingData() = default;

    bool operator<(const TgcCablingData&) const = delete;

    bool operator==(const TgcCablingData& other) const {
        return static_cast<const TgcCablingOfflineID&>(*this) == other &&
               static_cast<const TgcCablingReadoutID&>(*this) == other;
    }

};

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingOfflineID& obj);
std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingReadoutID& obj);
std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingData& obj);

}  // namespace MuonR4

#endif
