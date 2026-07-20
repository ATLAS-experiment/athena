#ifndef MUONCABLINGDATA_TGCCABLINGDATA_H
#define MUONCABLINGDATA_TGCCABLINGDATA_H

#include <cstdint>
#include <iostream>
#include <set>

#define CABLING_OPERATORS(CL_NAME)\
    bool operator<(const CL_NAME& other) const { return m_cache.hash < other.m_cache.hash; } \
    bool operator==(const CL_NAME& other) const { return m_cache.hash == other.m_cache.hash; } \
    bool operator!=(const CL_NAME& other) const { return m_cache.hash != other.m_cache.hash; } \
    bool operator!() const { return !m_cache.hash; }\
    CL_NAME() = default;\
    CL_NAME(const CL_NAME& other) : CL_NAME{} {\
        m_cache.hash = other.m_cache.hash;\
    }\
    CL_NAME& operator=(const CL_NAME& other) {\
        if (&other != this) m_cache.hash = other.m_cache.hash;\
        return *this;\
    }

namespace MuonR4 {

struct TgcCablingOfflineID {
    CABLING_OPERATORS(TgcCablingOfflineID)

    int8_t& stationName{m_cache.cache[0]};
    int8_t& stationEta{m_cache.cache[1]};
    int8_t& stationPhi{m_cache.cache[2]};
    int8_t& gasGap{m_cache.cache[3]};
    int8_t& isStrip{m_cache.cache[4]};
    int8_t& channel{m_cache.cache[5]};

private:
    union {
        int64_t hash{0};
        int8_t cache[8];
    } m_cache{};
};

struct TgcCablingReadoutID {
    CABLING_OPERATORS(TgcCablingReadoutID)

    int16_t& SLID{m_cache.cache[0]};
    int16_t& cellAddress1{m_cache.cache[1]};
    int16_t& cellAddress2{m_cache.cache[2]};
    int16_t& hitBitmap1{m_cache.cache[3]};
    int16_t& hitBitmap2{m_cache.cache[4]};

private:
    union {
        __int128_t hash{0};
        int16_t cache[8];
    } m_cache{};
};

struct TgcCablingData : public TgcCablingOfflineID, public TgcCablingReadoutID {
    TgcCablingData() = default;

    bool operator<(const TgcCablingData&) const = delete;

    bool operator==(const TgcCablingData& other) const {
        return static_cast<const TgcCablingOfflineID&>(*this) == other &&
               static_cast<const TgcCablingReadoutID&>(*this) == other;
    }

    bool operator!=(const TgcCablingData& other) const {
        return !((*this) == other);
    }
};

std::ostream& operator<<(std::ostream& ostr, const TgcCablingOfflineID& obj);
std::ostream& operator<<(std::ostream& ostr, const TgcCablingReadoutID& obj);
std::ostream& operator<<(std::ostream& ostr, const TgcCablingData& obj);

}  // namespace Muon

#undef CABLING_OPERATORS

#endif
