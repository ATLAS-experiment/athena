/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_RPCCANDDATA_H
#define L0MuonInterface_RPCCANDDATA_H

#include "L0MuonInterface/ICandData.h"

namespace L0Muon
{

  class RPCCandData : public ICandData
  {
  public:
    // default constructor
    RPCCandData() = default;
    ~RPCCandData() = default;

    RPCCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag);
    /// quality of the candidate
    enum class Quality
    {
      Q_UNDEFINED = 0,
      Q_BEST,
      Q_LOW
    };
    
    Quality quality() const { return m_quality; }
    float zPos(int index) const;
    uint8_t coinType() const;

    void setQuality(Quality quality) { m_quality = quality; }
    void setZPos(float zPos, int index);
    void setCoinType(uint8_t coinType);

    /// range of the RPC hits z positions
    static constexpr float s_zPosRange = 12500.0;
    /// range of the coincidence type value
    static constexpr uint8_t s_coinTypeRange = 6;
    /// 12 bits for z position
    static constexpr uint16_t s_zPosBitRange = 0xfff;
    /// 3 bits for the coincidence type
    static constexpr uint8_t s_coinTypeBitRange = 0x7;

  private:

    /// quality of the candidate
    Quality m_quality{0};
    /// Z positions of the RPC hits
    uint16_t m_zPos[4]{0, 0, 0, 0};
    /// coincidence type
    uint8_t m_coinType{0};
    
  };

}  // namespace L0Muon

#endif  // L0MuonInterface_RPCCANDDATA_H
