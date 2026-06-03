/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCID_H
#define MUONTGC_CABLING_TGCID_H

#include <type_traits>
#include <ostream>

namespace MuonTGC_Cabling {

class TGCId {
   public:
    enum class IdType { NoIdType, Channel, Module };

   public:
    TGCId(IdType vtype = IdType::NoIdType);
    virtual ~TGCId() = default;

    virtual bool isValid() const { return true; }

   public:
    // <internal numbering scheme>
    // 1. IdType
    // 2. IdIndex
    // int station [0..3]
    // int sectorRO [0..11]
    // int srod [0..2]
    // int octant  [0..7]
    // int sector  [0..47],[0..23]
    // int chamber [0..n]
    // int id      [0..n]
    //
    /// in ChannelId
    // int layer   [0..8]
    // int block   [0..n]
    // int channel [0..n]

    static constexpr int NUM_LAYERS =
        9;  // [0..2]:M1, [3..4]:M2, [5..6]:M3, [7..8]:M4(Inner)
    static constexpr int NUM_OCTANT = 8;
    static constexpr int NUM_ENDCAP_SECTOR = 48;
    static constexpr int NUM_FORWARD_SECTOR = 24;
    static constexpr int NUM_INNER_SECTOR = 24;
    static constexpr int N_RODS = 12;

    enum class SideType : int { Aside = 0, Cside = 1, MaxSideType = 2, Undefined = 99 };
    enum class StationType : int { M1 = 0, M2 = 1, M3 = 2, M4 = 3,    // M3 is also used for M2/M3 SLB
        MaxStationType = 4, Undefined = 99 };
    enum class ModuleType : int { WD = 0, SD = 1,
        WT = 2, ST = 3,
        WI = 4, SI = 5,
        MaxModuleType = 6,
        SL_SLB = 6,   // SL SLB module, not used in TGCId but in TGCModuleSLB
        Undefined = 99
    };
    enum class SignalType : int { Wire = 0, Strip = 1, MaxSignalType = 2, Undefined = 99 };
    enum class RegionType : int { Endcap = 0, Forward = 1, MaxRegionType = 2, Undefined = 99 };

    IdType getIdType() const;
    SideType getSideType() const;
    StationType getStation() const;
    ModuleType getModuleType() const;
    SignalType getSignalType() const;
    RegionType getRegionType() const;

    int getSectorInReadout() const;

    virtual int getSectorInOctant() const;
    virtual int getSectorModule() const;

    int getOctant() const;
    virtual int getSector() const;
    int getChamber() const;
    int getId() const;
    int getBlock() const;

    bool isAside() const;
    bool isCside() const;
    bool isStrip() const;
    bool isWire() const;
    bool isTriplet() const;
    bool isDoublet() const;
    bool isInner() const;
    bool isForward() const;
    bool isEndcap() const;

   public:
    void setSideType(SideType side);
    void setStation(StationType vstation);
    virtual void setModuleType(ModuleType module);
    void setSignalType(SignalType signal);
    void setRegionType(RegionType region);

    virtual void setOctant(int voctant);
    virtual void setSector(int vsector);
    virtual void setChamber(int chamber);
    void setId(int id);

   protected:
    void setIdType(IdType idtype);
    void setReadoutSector(int sector);
    void setSectorModule(int sectorModule);

   protected:
    SideType m_side{SideType::Undefined};
    StationType m_station{StationType::Undefined};
    ModuleType m_module{ModuleType::Undefined};
    SignalType m_signal{SignalType::Undefined};
    RegionType m_region{RegionType::Undefined};

    int m_octant{-1};
    int m_sector{-1};
    int m_chamber{-1};
    int m_id{-1};

   private:
    IdType m_idType{IdType::NoIdType};
};

inline TGCId::IdType TGCId::getIdType() const {
    return m_idType;
}
inline TGCId::SideType TGCId::getSideType() const {
    return m_side;
}
inline TGCId::StationType TGCId::getStation() const {
    return m_station;
}
inline TGCId::ModuleType TGCId::getModuleType() const {
    return m_module;
}
inline TGCId::SignalType TGCId::getSignalType() const {
    return m_signal;
}
inline TGCId::RegionType TGCId::getRegionType() const {
    return m_region;
}

inline int TGCId::getOctant() const {
    return m_octant;
}
inline int TGCId::getSector() const {
    return m_sector;
}
inline int TGCId::getChamber() const {
    return m_chamber;
}
inline int TGCId::getId() const {
    return m_id;
}

inline bool TGCId::isAside() const {
    return (m_side == SideType::Aside);
}
inline bool TGCId::isCside() const {
    return (m_side == SideType::Cside);
}
inline bool TGCId::isStrip() const {
    return (m_signal == SignalType::Strip);
}
inline bool TGCId::isWire() const {
    return (m_signal == SignalType::Wire);
}
inline bool TGCId::isTriplet() const {
    return (m_station == StationType::M1);
}
inline bool TGCId::isDoublet() const {
    return (m_station == StationType::M2 || m_station == StationType::M3);
}
inline bool TGCId::isInner() const {
    return (m_station == StationType::M4);
}
inline bool TGCId::isForward() const {
    return (m_region == RegionType::Forward);
}
inline bool TGCId::isEndcap() const {
    return (m_region == RegionType::Endcap);
}

inline void TGCId::setSideType(SideType side) {
    m_side = side;
}

inline void TGCId::setRegionType(RegionType region) {
    m_region = region;
}

inline void TGCId::setChamber(int chamber) {
    m_chamber = chamber;
}

inline void TGCId::setId(int id) {
    m_id = id;
}

inline void TGCId::setIdType(IdType idtype) {
    m_idType = idtype;
}

constexpr auto operator + (TGCId::SideType e) noexcept {
  return static_cast<std::underlying_type_t<TGCId::SideType>>(e);
}
constexpr auto operator + (TGCId::ModuleType e) noexcept {
  return static_cast<std::underlying_type_t<TGCId::ModuleType>>(e);
}
constexpr auto operator + (TGCId::SignalType e) noexcept {
  return static_cast<std::underlying_type_t<TGCId::SignalType>>(e);
}
constexpr auto operator + (TGCId::RegionType e) noexcept {
  return static_cast<std::underlying_type_t<TGCId::RegionType>>(e);
}

inline std::ostream& operator<<(std::ostream& os, const TGCId::SideType& type) {
    return os << (type == TGCId::SideType::Aside ? "Aside" : "Cside");
}
inline std::ostream& operator<<(std::ostream& os, const TGCId::StationType& type) {
    return os << static_cast<int>(type);
}
inline std::ostream& operator<<(std::ostream& os, const TGCId::ModuleType& type) {
    return os << static_cast<int>(type);
}
inline std::ostream& operator<<(std::ostream& os, const TGCId::SignalType& type) {
    return os << (type == TGCId::SignalType::Wire ? "Wire" : "Strip");
}
inline std::ostream& operator<<(std::ostream& os, const TGCId::RegionType& type) {
    return os << (type == TGCId::RegionType::Endcap ? "Endcap" : "Forward");
}

}  // namespace MuonTGC_Cabling

#endif
