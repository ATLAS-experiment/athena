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
     * @brief Retrieve the NSW segments
     * @return NSW segments
     */
    uint32_t nswSegment() const;

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

    /// Bit mask for deltaPhi : 1 bit for sign and 3 bits
    static constexpr uint8_t DPHI_BIT_MASK = 0x7;
    /// Bit mask for deltaTheta : 1 bit for sign and 6 bits
    static constexpr uint8_t DTHETA_BIT_MASK = 0x3f;
    /// Bit mask for NSW-TP output
    static constexpr uint32_t NSW_BIT_MASK = 0xfffffff;

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::TGCCandData_v1, SG::AuxElement );

#endif  // XAODL0MUONCAND_VERSION_TGCCANDDATA_V1_H