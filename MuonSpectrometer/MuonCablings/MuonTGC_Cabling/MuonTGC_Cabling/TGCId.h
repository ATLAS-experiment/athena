/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCID_H
#define MUONTGC_CABLING_TGCID_H

namespace MuonTGC_Cabling {
 
class TGCId {
 public:
  enum class IdType { NoIdType, Channel, Module, MaxIdType };

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

  static constexpr int NUM_STATIONS = 4;
  static constexpr int NUM_LAYERS = 9;  // [0..2]:M1, [3..4]:M2, [5..6]:M3, [7..8]:M4(Inner)
  static constexpr int NUM_OCTANT = 8;
  static constexpr int NUM_ENDCAP_SECTOR = 48;
  static constexpr int NUM_FORWARD_SECTOR = 24;
  static constexpr int NUM_INNER_SECTOR = 24;
  static constexpr int N_RODS = 12;

  enum SideType      {NoSideType=-1,      Aside,Cside,
                      MaxSideType};
  enum ModuleType    {NoModuleType=-1,    WD,SD,WT,ST,WI,SI,
                      MaxModuleType};
  enum SignalType    {NoSignalType=-1,    Wire,Strip,
                      MaxSignalType};
  enum MultipletType {NoMultipletType=-1, Doublet,Triplet,Inner,
                      MaxChamberType};
  enum RegionType    {NoRegionType=-1,    Endcap,Forward,
                      MaxRegionType};

  IdType getIdType (void) const;
  SideType    getSideType (void) const;
  ModuleType  getModuleType (void) const;
  SignalType  getSignalType (void) const;
  MultipletType  getMultipletType (void) const;
  RegionType  getRegionType (void) const;

  int getSectorInReadout(void) const;
  
  virtual int getSectorInOctant() const;
  virtual int getSectorModule() const;

  int getStation() const;
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
  void setModuleType(ModuleType module);
  void setSignalType(SignalType signal);
  void setMultipletType(MultipletType multiplet);
  void setRegionType(RegionType region);

  virtual void setStation (int vstation);
  virtual void setOctant (int voctant);
  virtual void setSector (int vsector);
  virtual void setChamber(int chamber);
  void setId(int id);

 protected:
  void setIdType(IdType idtype);
  void setReadoutSector(int sector);
  void setSectorModule(int sectorModule);

 protected:
  SideType      m_side{NoSideType};
  ModuleType    m_module{NoModuleType};
  SignalType    m_signal{NoSignalType};
  MultipletType m_multiplet{NoMultipletType};
  RegionType    m_region{NoRegionType};

  int m_station{-1};
  int m_octant{-1};
  int m_sector{-1};
  int m_chamber{-1};
  int m_id{-1};

 private:
  IdType m_idType{IdType::NoIdType};
};

inline TGCId::IdType        TGCId::getIdType()        const { return m_idType; }
inline TGCId::SideType      TGCId::getSideType()      const { return m_side; }
inline TGCId::ModuleType    TGCId::getModuleType()    const { return m_module; }
inline TGCId::SignalType    TGCId::getSignalType()    const { return m_signal; }
inline TGCId::MultipletType TGCId::getMultipletType() const { return m_multiplet; }
inline TGCId::RegionType    TGCId::getRegionType()    const { return m_region; }

inline int TGCId::getStation() const { return m_station; }
inline int TGCId::getOctant()  const { return m_octant; }
inline int TGCId::getSector()  const { return m_sector; }
inline int TGCId::getChamber() const { return m_chamber; }
inline int TGCId::getId()      const { return m_id; }

inline bool TGCId::isAside()   const { return (m_side == Aside); }
inline bool TGCId::isCside()   const { return (m_side == Cside); }
inline bool TGCId::isStrip()   const { return (m_signal == Strip); }
inline bool TGCId::isWire()    const { return (m_signal == Wire); }
inline bool TGCId::isTriplet() const { return (m_multiplet == Triplet); }
inline bool TGCId::isDoublet() const { return (m_multiplet == Doublet); }
inline bool TGCId::isInner()   const { return (m_multiplet == Inner); }
inline bool TGCId::isForward() const { return (m_region == Forward); }
inline bool TGCId::isEndcap()  const { return (m_region == Endcap); }

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

}  // end of namespace
 
#endif
