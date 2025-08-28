/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_TGCCANDDATA_H
#define L0MuonInterface_TGCCANDDATA_H

#include "L0MuonInterface/ICandData.h"

namespace L0Muon {

class TGCCandData : public ICandData {
 public:
  TGCCandData() = default;
  ~TGCCandData() = default;

  TGCCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
  : ICandData(subdetectorId, sectorId, bcTag) {}

  uint8_t coinType() const;
  float deltaPhi() const;
  float deltaTheta() const;
  uint32_t nswSegment() const;

  void setCoinType(uint8_t cointype);
  void setDeltaPhi(float dphi);
  void setDeltaTheta(float dtheta);
  void setNswSegment(uint32_t nswout);

  /// range of the RPC hits z positions
  static constexpr float s_dthetaRange = 0.160;   // radian
  static constexpr float s_dphiRange = 0.032;     // radian

  /// Bit mask for Coincidence Types
  static constexpr uint8_t COINTYPE_BIT_MASK = 0x7;
  /// Bit mask for deltaPhi : 1 bit for sign and 3 bits
  static constexpr uint8_t DPHI_BIT_MASK = 0x7;
  /// Bit mask for deltaTheta : 1 bit for sign and 6 bits
  static constexpr uint8_t DTHETA_BIT_MASK = 0x3f;
  /// Bit mask for NSW-TP output
  static constexpr uint32_t NSW_BIT_MASK = 0xfffffff;

 private:
  /// Coincidence Type (3 bits) [rsv.][GoodMF][InnerCoin]
  uint8_t m_coinType{0};

  /// Segment azimuthal angle w.r.t. the vector from IP to the segment position (4 bits)
  uint8_t m_deltaPhi{0};

  /// Segment polar angle w.r.t. the vector from IP to the segment position (7 bits)
  uint8_t m_deltaTheta{0};

  /// Copy of the NSW-TP output (To be defined)
  uint32_t m_nswSegment{0};

};

}  // namespace L0Muon

#endif  // L0MuonInterface_TGCCANDDATA_H
