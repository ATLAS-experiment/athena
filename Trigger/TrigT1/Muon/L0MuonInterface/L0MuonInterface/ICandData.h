/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_ICANDDATA_H
#define L0MuonInterface_ICANDDATA_H

#include <cstdint>

namespace L0Muon
{

  class ICandData
  {
  public:

    // default constructor
    ICandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
      : m_subdetectorId(subdetectorId), m_sectorId(sectorId), m_bcTag(bcTag) {}

    virtual ~ICandData() = default;

    uint16_t subdetectorId() const { return m_subdetectorId; };
    uint16_t sectorId() const { return m_sectorId; };
    uint16_t bcTag() const { return m_bcTag; };
    uint16_t eta() const { return m_eta; };
    uint16_t phi() const { return m_phi; };
    uint16_t pt() const { return m_pt; };
    uint8_t threshold() const { return m_threshold; };
    uint8_t charge() const { return m_charge; };

    /// Set functions of the modifiable parameters
    void setEta(uint16_t eta) { m_eta = eta; }
    void setPhi(uint16_t phi) { m_phi = phi; }
    void setPt(uint16_t pt) { m_pt = pt; }
    void setThreshold(uint8_t threshold) { m_threshold = threshold; }
    void setCharge(uint8_t charge) { m_charge = charge; }
    
    enum class BC_ID
    {
      BC_UNDEFINED = 0,
      BC_PREVIOUS,
      BC_CURRENT,
      BC_NEXT,
      BC_NEXTNEXT
    };

  private:
    // number of the subdetector  
    uint16_t m_subdetectorId{0};  
    /// sector number
    uint16_t m_sectorId{0};
    /// BC tag
    uint16_t m_bcTag{0};
    /// theta coordinate of the candidate
    uint16_t m_eta{0};
    /// phi coordinate of the candidate
    uint16_t m_phi{0};
    /// pt of the candidate
    uint16_t m_pt{0};
    /// threshold
    uint8_t m_threshold{0};
    /// charge
    uint8_t m_charge{0};
  };
}

#endif