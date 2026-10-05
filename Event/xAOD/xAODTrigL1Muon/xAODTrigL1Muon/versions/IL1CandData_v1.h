
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1MUON_VERSION_IL1CANDDATA_V1_H
#define XAODTRIGL1MUON_VERSION_IL1CANDDATA_V1_H

#include <cstdint>
#include <cmath>
#include "AthContainers/AuxElement.h"

namespace xAOD {

  /** @brief  base class providing a common interface with shared variables for the L0 muon candidates from RPC/TGC SL to MDT-TP
  */

  class IL1CandData_v1 : public SG::AuxElement
  {
  public:
    /// Default constructor
    IL1CandData_v1() = default;
    virtual ~IL1CandData_v1() = default;


    /// Bunch crossing identifier
    enum class BC_ID
    {
      BC_UNDEFINED = 0,
      BC_PREVIOUS,
      BC_CURRENT,
      BC_NEXT,
      BC_NEXTNEXT
    };

    /**
    * @brief Retrieve the sub detector id
    * @return Subdetector ID
    */
    uint16_t l1SubdetectorId() const;
    /**
     * @brief Retrieve the sector id
     * @return Sector ID
     */
    uint16_t l1SectorId() const;
    /**
     * @brief Retrieve the bunch crossing tag
     * @return Bunch crossing tag
     */
    uint16_t bcTag() const;
    /**
     * @brief Retrieve the threshold
     * @return Threshold
     */
    uint8_t l1Threshold() const;
    /**
     * @brief Retrieve the candidate charge
     * @return Candidate charge
     */
    uint8_t l1CandCharge() const;
    /**
     * @brief Retrieve the coincidence type
     * @return Coincidence type
     */
    uint8_t coinType() const;

    /**
     * @brief Retrieve the eta
     * @return Eta
     */
    uint16_t l1Eta() const;
    /**
     * @brief Retrieve the phi
     * @return Phi
     */
    uint16_t l1Phi() const;
    /**
     * @brief Retrieve the encoded candidate pT
     * @return Eight-bit pT code in 0.5 GeV steps; zero denotes an invalid pT estimate
     */
    uint8_t l1Pt() const;
    /**
     * @brief Retrieve the trigger-candidate identifier
     * @return Three-bit trigger-candidate identifier; zero denotes an empty candidate
     */
    uint8_t tcId() const;

    /**
     * @brief Set the sub detector id
     * @param subdetectorId Subdetector ID
     */
    void setL1SubdetectorId(uint16_t subdetectorId);
    /**
     * @brief Set the sector id
     * @param sectorId Sector ID
     */
    void setL1SectorId(uint16_t sectorId);
    /**
     * @brief Set the bunch crossing tag
     * @param bcTag Bunch crossing tag
     */
    void setBcTag(uint16_t bcTag);
    /**
     * @brief Set the eta
     * @param eta Eta
     */
    void setL1Eta(float eta);
    /**
     * @brief Set the phi
     * @param phi Phi
     */
    void setL1Phi(float phi);
    /**
     * @brief Set the candidate pT
     * @param pt Candidate pT in GeV, encoded in 0.5 GeV steps
     */
    void setL1Pt(float pt);
    /**
     * @brief Set the threshold
     * @param threshold Threshold
     */
    void setL1Threshold(uint8_t threshold);
    /**
     * @brief Set the candidate charge
     * @param candCharge Candidate charge
     */
    void setL1CandCharge(uint8_t candCharge);
      /**
      * @brief Set the coincidence type
      * @param coinType Coincidence type
      */
    void setCoinType(uint8_t coinType);
    /**
     * @brief Set the trigger-candidate identifier
     * @param value Three-bit trigger-candidate identifier; zero denotes an empty candidate
     */
    void setTcId(uint8_t value);

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
    static constexpr uint8_t tcIdBitMask() { return TC_ID_BIT_MASK; }

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
    /// Bit mask for the trigger-candidate identifier
    static constexpr uint8_t TC_ID_BIT_MASK = 0x7;
  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::IL1CandData_v1, SG::AuxElement );

#endif  // XAODTRIGL1MUON_VERSION_IL1CANDDATA_V1_H
