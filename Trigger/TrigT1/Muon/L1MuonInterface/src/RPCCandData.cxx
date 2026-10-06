/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "L1MuonInterface/RPCCandData.h"
#include <algorithm>
#include <stdexcept>

namespace L1Muon
{

  RPCCandData::RPCCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
      : ICandData(subdetectorId, sectorId, bcTag)

  {
    // Initialize the z positions and coincidence type
    for (int i = 0; i < 4; ++i)
    {
      m_zPos[i] = 0xffff;
    }
    m_coinType = 0;
  }

  void RPCCandData::setZPos(float zPos, int index)
  {
    if (index < 0 || index > 3)
    {
      return;
    }
    /// convert z position to binary

    m_zPos[index] = static_cast<uint16_t>(std::round((zPos + s_zPosRange) / (2.0f * s_zPosRange) * static_cast<float>(s_zPosBitRange)));

   
  }
  void RPCCandData::setCoinType(uint8_t coinType)
  {
    /// convert the coincidence type to binary
    m_coinType = static_cast<uint8_t>(coinType / s_coinTypeRange * static_cast<float>(s_coinTypeBitRange));
    }


  float RPCCandData::zPos(int index) const
{
  if (index < 0 || index > 3) {
    return 0.0;
  }

  if (m_zPos[index] == 0xffff) {
    return -999.f;
  }
  return static_cast<float>(m_zPos[index]) / static_cast<float>(s_zPosBitRange) * 2.0f * s_zPosRange - s_zPosRange;
}

  uint8_t RPCCandData::coinType() const
  {
    return static_cast<uint8_t>(m_coinType / s_coinTypeRange * static_cast<float>(s_coinTypeBitRange));
  }
  
}

