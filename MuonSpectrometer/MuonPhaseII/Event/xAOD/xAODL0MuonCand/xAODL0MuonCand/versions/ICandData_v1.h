
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODL0MUONCAND_VERSION_ICANDDATA_V1_H
#define XAODL0MUONCAND_VERSION_ICANDDATA_V1_H

#include <cstdint>
#include <cmath>
#include "AthContainers/AuxElement.h"

namespace xAOD
{

  /** @brief  base class providing a common interface with shared variables for the L0 muon candidates from RPC/TGC SL to MDT-TP
  */

  class ICandData_v1 : public SG::AuxElement
  {
  public:
    /// Default constructor
    ICandData_v1() = default;
    virtual ~ICandData_v1() = default;


    /// Bunch crossing identifier
    enum class BC_ID
    {
      BC_UNDEFINED = 0,
      BC_PREVIOUS,
      BC_CURRENT,
      BC_NEXT,
      BC_NEXTNEXT
    };
      // quality of the candidate
    enum class Quality : uint8_t
    {
      Q_UNDEFINED = 0,
      Q_BEST,
      Q_LOW
    };

    /**
    * @brief Retrieve the sub detector id 
    * @return Subdetector ID
    */
    uint16_t subdetectorId() const;
    /**
     * @brief Retrieve the sector id 
     * @return Sector ID
     */
    uint16_t sectorId() const;
    /**
     * @brief Retrieve the bunch crossing tag 
     * @return Bunch crossing tag
     */
    uint16_t bcTag() const;
    /**
     * @brief Retrieve the threshold 
     * @return Threshold
     */
    uint8_t threshold() const;
    /**
     * @brief Retrieve the candidate charge 
     * @return Candidate charge
     */
    uint8_t candCharge() const;
    /**
     * @brief Retrieve the MDT flag 
     * @return MDT flag
     */
    uint8_t mdtFlag() const;
    /**
     * @brief Retrieve the coincidence type 
     * @return Coincidence type
     */
    uint8_t coinType() const;

    /**
     * @brief Retrieve the eta 
     * @return Eta
     */
    uint16_t eta() const;
    /**
     * @brief Retrieve the phi 
     * @return Phi
     */
    uint16_t phi() const;
    /**
     * @brief Retrieve the encoded candidate pT
     * @return Eight-bit pT code in 0.5 GeV steps; zero denotes an invalid pT estimate
     */
    uint8_t pt() const;
    /**
     * @brief Retrieve the candidate quality 
     * @return Candidate quality
     */
    Quality candQuality() const;

  

    /**
     * @brief Set the sub detector id 
     * @param subdetectorId Subdetector ID
     */
    void setSubdetectorId(uint16_t subdetectorId);
    /**
     * @brief Set the sector id 
     * @param sectorId Sector ID
     */
    void setSectorId(uint16_t sectorId);
    /**
     * @brief Set the bunch crossing tag 
     * @param bcTag Bunch crossing tag
     */
    void setBcTag(uint16_t bcTag);
    /**
     * @brief Set the eta 
     * @param eta Eta
     */
    void setEta(float eta);
    /**
     * @brief Set the phi 
     * @param phi Phi
     */
    void setPhi(float phi);
    /**
     * @brief Set the candidate pT
     * @param pt Candidate pT in GeV, encoded in 0.5 GeV steps
     */
    void setPt(float pt);
    /**
     * @brief Set the threshold 
     * @param threshold Threshold
     */
    void setThreshold(uint8_t threshold);
    /**
     * @brief Set the candidate charge 
     * @param candCharge Candidate charge
     */
    void setCandCharge(uint8_t candCharge);
    /**
     * @brief Set the MDT flag 
     * @param mdtFlag MDT flag
     */
    void setMdtFlag(uint8_t mdtFlag);
      /**
      * @brief Set the coincidence type 
      * @param coinType Coincidence type
      */
    void setCoinType(uint8_t coinType);
    /**
     * @brief Set the candidate quality 
     * @param candQuality Candidate quality
     */
    void setCandQuality(Quality candQuality);

    /// Initialize candidate with basic properties
      /**
    * @brief Initialise the object given some input data
    * @param subdetectorId Subdetector ID
    * @param sectorId Sector ID
    * @param bcTag Bunch crossing tag
    */

    void initialize(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag);

    
    static constexpr float etaRange() { return s_etaRange; }
    static constexpr uint16_t etaBitRange() { return s_etaBitRange; }
    static constexpr float phiRange() { return s_phiRange; }
    static constexpr uint16_t phiBitRange() { return s_phiBitRange; }
    static constexpr float ptResolution() { return s_ptResolution; }
    static constexpr float ptRange() { return s_ptRange; }
    static constexpr uint8_t ptBitRange() { return s_ptBitRange; }
    static constexpr uint8_t coinTypeBitMask() { return COINTYPE_BIT_MASK; }

  protected:
    /// Variables range
    static constexpr float s_etaRange = 2.7;
    static constexpr float s_phiRange = 2.0 * M_PI;
    static constexpr float s_ptResolution = 0.5F;  ///< GeV per pT code

    /// Variables bit size
    /// 14 bits for eta, 9 bits for phi, 8 bits for pt
    static constexpr uint16_t s_etaBitRange = 0x3fff;
    static constexpr uint16_t s_phiBitRange = 0x1ff;
    static constexpr uint8_t s_ptBitRange = 0xff;
    static constexpr float s_ptRange =
        s_ptResolution * static_cast<float>(s_ptBitRange);

    /// Bit mask for Coincidence Types
    static constexpr uint8_t COINTYPE_BIT_MASK = 0x7;
  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::ICandData_v1, SG::AuxElement );

#endif  // XAODL0MUONCAND_VERSION_ICANDDATA_V1_H
