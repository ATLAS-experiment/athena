/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "L0MuonInterface/BarrelCandData.h"

namespace L0Muon
{

  BarrelCandData::BarrelCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
      : ICandData(subdetectorId, sectorId, bcTag)

  {
    // Initialize the z positions and coincidence type
    for (int i = 0; i < 4; ++i)
    {
      m_zPos[i] = 0xffff;
    }
    m_coinType = 0;
  }

  void BarrelCandData::setZPos(float zPos, int index)
  {
    if (index < 0 || index > 3)
    {
      return;
    }
    /// convert z position to binary
    m_zPos[index] = (uint16_t)(zPos / s_zPosRange * (float)s_zPosBitRange);
  }
  void BarrelCandData::setCoinType(uint8_t coinType)
  {
    /// convert the coincidence type to binary
    m_coinType = (uint8_t)(coinType / s_coinTypeRange * (float)s_coinTypeBitRange);
  }

  float BarrelCandData::zPos(int index) const
  {
    if (index < 0 || index > 3)
    {
      return 0.0;
    }
    return (float)m_zPos[index] / (float)s_zPosBitRange * s_zPosRange;
  }
  uint8_t BarrelCandData::coinType() const
  {
    return (uint8_t)(m_coinType / s_coinTypeRange * (float)s_coinTypeBitRange);
  }
  
}
