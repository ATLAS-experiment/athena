/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODL0MUONCAND_VERSION_TGCCANDDATA_V1_H
#define XAODL0MUONCAND_VERSION_TGCCANDDATA_V1_H

// System include(s):
#include <cstdint>

#include "AthContainers/AuxElement.h"
#include "xAODL0MuonCand/ICandData.h"

namespace xAOD
{

  /** @brief Data class describing the L0 muon candidates from TGC-SL to MDT-TP
  */

  class TGCCandData_v1 : public ICandData_v1 {
  public:
    // default constructor and destructor
    TGCCandData_v1() = default;
    ~TGCCandData_v1() = default;

    /**
     * @brief Retrieve the trigger-candidate identifier
     * @return Three-bit trigger-candidate identifier; zero denotes an empty candidate
     */
    uint8_t tcId() const;
    /**
     * @brief Retrieve the passed pT-threshold index
     * @return Four-bit pT-threshold index
     */
    uint8_t passedPtThresholdIndex() const;
    /**
     * @brief Retrieve the software-side encoded TGC pT estimate
     * @return Encoded TGC pT-value index
     */
    uint8_t estimatedPtValueIndex() const;
    /**
     * @brief Retrieve the software-side TGC pT estimate in GeV
     * @return TGC pT estimate in 0.5 GeV bins
     */
    float ptValueGeV() const;
    /**
     * @brief Check whether the software-side encoded TGC pT estimate is valid
     * @return True when the encoded pT estimate is valid
     */
    bool estimatedPtValueValid() const;
    /**
     * @brief Retrieve the detector side
     * @return True for side A and false for side C
     */
    bool side() const;
    /**
     * @brief Retrieve the endcap flag
     * @return One-bit endcap flag
     */
    uint8_t endcapFlag() const;
    /**
     * @brief Retrieve the TGC trigger-sector number
     * @return Six-bit sector number
     */
    uint8_t sector() const;
    /**
     * @brief Check whether the candidate passed Inner Coincidence
     * @return True when the Inner Coincidence bit is set in CoinType
     */
    bool hasInnerCoincidence() const;
    /**
     * @brief Check whether the candidate is in a good magnetic-field region
     * @return True when the GoodMag bit is set in CoinType
     */
    bool goodMagneticField() const;
    /**
     * @brief Retrieve the delta phi wrt vector from IP to segment position
     * @return Delta phi value
     */
    float deltaPhi() const; // 4 bits, [2:0] the absolute value in [0,0.032] and [3] the sign (0,1)
    /**
     * @brief Retrieve the delta theta value wrt vector from IP to segment position
     * @return Delta theta value
     */
    float deltaTheta() const; // 7 bits, [5:0] the absolute value in [0,0.160]mrad and [6] the sign (0,1)
    /**
     * @brief Retrieve the raw encoded delta phi word
     * @return Four-bit signed delta phi word
     */
    uint8_t deltaPhiWord() const;
    /**
     * @brief Retrieve the raw encoded delta theta word
     * @return Seven-bit signed delta theta word
     */
    uint8_t deltaThetaWord() const;
    /**
     * @brief Retrieve the NSW segments
     * @return NSW segments
     */
    uint32_t nswSegment() const;

    /**
     * @brief Set the trigger-candidate identifier
     * @param value Three-bit trigger-candidate identifier; zero denotes an empty candidate
     */
    void setTcId(uint8_t value);
    /**
     * @brief Set the passed pT-threshold index
     * @param value Four-bit pT-threshold index
     */
    void setPassedPtThresholdIndex(uint8_t value);
    /**
     * @brief Set the software-side encoded TGC pT estimate
     * @param value Encoded TGC pT-value index
     */
    void setEstimatedPtValueIndex(uint8_t value);
    /**
     * @brief Set the software-side encoded TGC pT estimate validity
     * @param value True when the encoded pT estimate is valid
     */
    void setEstimatedPtValueValid(bool value);
    /**
     * @brief Set the detector side
     * @param value True for side A and false for side C
     */
    void setSide(bool value);
    /**
     * @brief Set the endcap flag
     * @param value One-bit endcap flag
     */
    void setEndcapFlag(uint8_t value);
    /**
     * @brief Set the TGC trigger-sector number
     * @param value Six-bit sector number
     */
    void setSector(uint8_t value);
    /**
     * @brief Set the Inner Coincidence bit in CoinType
     * @param value New state of the Inner Coincidence bit
     */
    void setHasInnerCoincidence(bool value);
    /**
     * @brief Set the GoodMag bit in CoinType
     * @param value New state of the GoodMag bit
     */
    void setGoodMagneticField(bool value);
    /**
     * @brief Set the delta phi value
     * @param dphi Delta phi value
     */
    void setDeltaPhi(float dphi);
    /**
     * @brief Set the delta theta value
     * @param dtheta Delta theta value
     */
    void setDeltaTheta(float dtheta);
    /**
     * @brief Set the raw encoded delta phi word
     * @param value Four-bit signed delta phi word
     */
    void setDeltaPhiWord(uint8_t value);
    /**
     * @brief Set the raw encoded delta theta word
     * @param value Seven-bit signed delta theta word
     */
    void setDeltaThetaWord(uint8_t value);
    /**
     * @brief Set the NSW segments
     * @param nswout NSW segments
     */
    void setNswSegment(uint32_t nswout);

  private:
    /// range of the RPC hits z positions
    static constexpr float s_dthetaRange = 0.160;   // radian
    static constexpr float s_dphiRange = 0.032;     // radian
    /// range of the TGC hits positions
    static constexpr float s_posRange = 12500.0F;

    /// CoinType bit for Inner Coincidence
    static constexpr uint8_t COINTYPE_INNER_COINCIDENCE_BIT = 0x1;
    /// CoinType bit for the GoodMag flag
    static constexpr uint8_t COINTYPE_GOOD_MAGNETIC_FIELD_BIT = 0x2;
    /// Bit mask for deltaPhi : 1 bit for sign and 3 bits
    static constexpr uint8_t DPHI_BIT_MASK = 0x7;
    /// Bit mask for deltaTheta : 1 bit for sign and 6 bits
    static constexpr uint8_t DTHETA_BIT_MASK = 0x3f;
    /// Bit mask for NSW-TP output
    static constexpr uint32_t NSW_BIT_MASK = 0xfffffff;
    /// Bit mask for the trigger-candidate identifier
    static constexpr uint8_t TC_ID_BIT_MASK = 0x7;
    /// Bit mask for the passed pT-threshold index
    static constexpr uint8_t PT_THRESHOLD_BIT_MASK = 0xf;

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::TGCCandData_v1, xAOD::ICandData_v1 );

#endif  // XAODL0MUONCAND_VERSION_TGCCANDDATA_V1_H
