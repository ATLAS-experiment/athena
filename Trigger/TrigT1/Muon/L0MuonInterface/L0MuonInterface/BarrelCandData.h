/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_BARRELCANDDATA_H
#define L0MuonInterface_BARRELCANDDATA_H

#include "L0MuonInterface/ICandData.h"

namespace L0Muon
{

  class BarrelCandData : public ICandData
  {
  public:
    // default constructor
    BarrelCandData() = default;
    ~BarrelCandData() = default;

    BarrelCandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag);

    uint16_t quality() const { return m_quality; }
    void setQuality(uint16_t quality) { m_quality = quality; }

    /// quality of the candidate
    enum class Quality
    {
      Q_UNDEFINED = 0,
      Q_BEST,
      Q_LOW
    };

  private:

    /// quality of the candidate
    uint16_t m_quality{0};
    
  };

} // namespace L0Muon

#endif // L0MuonInterface_BARRELCANDDATA_H
