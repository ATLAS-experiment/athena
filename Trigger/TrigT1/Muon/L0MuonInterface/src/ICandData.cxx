/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "L0MuonInterface/ICandData.h"

namespace L0Muon
{

  /// set the kinematic parameters using the 
  /// granularity of the binary representation
  void ICandData::setEta(float eta)
  {
    /// convert eta to binary
    m_eta = (uint16_t)(eta / s_etaRange * (float)s_etaBitRange);
  }
  void ICandData::setPhi(float phi)
  {
    m_phi = (uint16_t)(phi / s_phiRange * (float)s_phiBitRange);
  }
  void ICandData::setPt(float pt)
  {
    m_pt = (uint16_t)(pt / s_ptRange * (float)s_ptBitRange);
  }

  /// get the kinematic parameters
  float ICandData::eta() const
  {
    return (float)m_eta / (float)s_etaBitRange * s_etaRange;
  }
  float ICandData::phi() const
  {
    return (float)m_phi / (float)s_phiBitRange * s_phiRange;
  }
  float ICandData::pt() const
  {
    return (float)m_pt / (float)s_ptBitRange * s_ptRange;
  }

} // namespace L0Muon