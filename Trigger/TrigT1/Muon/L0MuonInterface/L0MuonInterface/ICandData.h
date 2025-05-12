/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_ICANDDATA_H
#define L0MuonInterface_ICANDDATA_H

#include <cstdint>
#include <cmath>

namespace L0Muon
{

  class ICandData
  {
  public:
    // default constructor
    ICandData(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag)
        : m_subdetectorId(subdetectorId), m_sectorId(sectorId), m_bcTag(bcTag) {}

    ICandData() = default;
    virtual ~ICandData() = default;

    uint16_t subdetectorId() const { return m_subdetectorId; };
    uint16_t sectorId() const { return m_sectorId; };
    uint16_t bcTag() const { return m_bcTag; };
    uint8_t threshold() const { return m_threshold; };
    uint8_t charge() const { return m_charge; };
    uint8_t mdtFlag() const { return m_mdtFlag; };
    /// get the kinematic parameters
    float eta() const;
    float phi() const;
    float pt() const;

    /// Set functions of the modifiable parameters
    void setEta(float eta);
    void setPhi(float phi);
    void setPt(float pt);
    void setThreshold(float threshold) { m_threshold = threshold; }
    void setCharge(uint8_t charge) { m_charge = charge; }
    void setMdtFlag(uint8_t mdtFlag) { m_mdtFlag = mdtFlag; }

    enum class BC_ID
    {
      BC_UNDEFINED = 0,
      BC_PREVIOUS,
      BC_CURRENT,
      BC_NEXT,
      BC_NEXTNEXT
    };

  private:
    /// variables range
    static constexpr float s_etaRange = 2.7;
    static constexpr float s_phiRange = 2.0 * M_PI;
    static constexpr float s_ptRange = 1000.0;

    /// variables bit size
    /// 14 bits for eta, 9 bits for phi, 8 bits for pt
    static constexpr uint16_t s_etaBitRange = 0x3fff;
    static constexpr uint16_t s_phiBitRange = 0x1ff;
    static constexpr uint16_t s_ptBitRange = 0xff;

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
    /// charge ( 0=negative, 1=positive)
    uint8_t m_charge{0};
    /// MDT flag
    uint8_t m_mdtFlag{0};
  };
}

#endif