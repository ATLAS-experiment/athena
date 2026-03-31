/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "L0MuonInterface/ICandData.h"

#include <iostream>

namespace L0Muon
{
    
    /// set the kinematic parameters using the 
    /// granularity of the binary representation
    void ICandData::setEta(float eta)
    {
        /// convert eta to binary, taking into account the range from -s_etaRange to +s_etaRange) 
        m_eta = static_cast<uint16_t>(std::round((eta+s_etaRange)/(2.0f*s_etaRange)*static_cast<float>(s_etaBitRange)));
    }
    void ICandData::setPhi(float phi)
    {
        m_phi = static_cast<uint16_t>(((phi+M_PI)/s_phiRange)*static_cast<float>(s_phiBitRange));
    }
    void ICandData::setPt(float pt)
    {
        m_pt = static_cast<uint16_t>(std::round(pt/s_ptRange)*static_cast<float>(s_ptBitRange));
    }

  /// get the kinematic parameters
  float ICandData::eta() const
  {
    /// return eta with +/- sign from -s_etaRange to +s_etaRange
      return static_cast<float>(m_eta)/static_cast<float>(s_etaBitRange)*2.0f* s_etaRange - s_etaRange;
  }
  float ICandData::phi() const
  {
      return static_cast<float> (m_phi) / static_cast<float>(s_phiBitRange) * s_phiRange-M_PI;
  }
  float ICandData::pt() const
  {
      return static_cast<float>(m_pt)/static_cast<float>(s_ptBitRange)*s_ptRange;
  }

} // namespace L0Muon
