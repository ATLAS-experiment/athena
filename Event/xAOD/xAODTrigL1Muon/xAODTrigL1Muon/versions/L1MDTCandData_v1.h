/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1MUON_VERSION_L1MDTCANDDATA_V1_H
#define XAODTRIGL1MUON_VERSION_L1MDTCANDDATA_V1_H

// System include(s):
#include <cstdint>

#include "AthContainers/AuxElement.h"
#include "xAODTrigL1Muon/IL1CandData.h"

namespace xAOD {

    /** @brief Data class describing the L0 muon candidates from MDT-TP to TGC/RPC SL
  */

  class L1MDTCandData_v1 : public IL1CandData_v1 {
  public:
    // default constructor and destructor
    L1MDTCandData_v1() = default;
    ~L1MDTCandData_v1() = default;

    /// Quality flag for each segment
    enum class SegmentQuality : uint8_t
    {
      Q_UNDEFINED = 0,
      Q_BEST,
      Q_LOW
    };

    ///// getters
    /**
     * @brief Retrieve the MDT flag
     * @return MDT flag
     */
    uint8_t l1MdtFlag() const;
    /**
     * @brief Retrieve the number of MDT segments asspcoate to the muon candidate
     * @return Number of segments
     */
    uint8_t l1NumSegments() const; // 2 bits, range [0:3]
    /**
     * @brief Retrieve the segment quality flag for each segment
     * @return Segment quality flag
     */
    uint8_t l1SegmentQualityFlag() const;  //3 bits, segment quality flags ( [0:1] per segment)
    /**
     * @brief Retrieve the charge value from the sector logic
     * @return sector logic charge
     */
    uint8_t l1SlCharge() const; // 1 bit, range [0:1]
    /**
     * @brief Retrieve the highest RPC/TGC pT threshold satisfied in sector logic
     * @return sector logic pT threshold
     */
    uint8_t l1SlPtThreshold() const; // 4 bits, highest RPC/TGC pT threshold satisfied (15 levels)
    /**
     * @brief Retrieve the phi position in global coordinates from the sector logic
     * @return sector logic phi position
     */
    uint16_t l1SlPhiPosition() const; // 9 bits, range [0,2PI]
    /**
     * @brief Retrieve the eta position in global coordinates from the sector logic
     * @return sector logic eta position
     */
    uint16_t l1SlEtaPosition() const; // 14 bits, range [-2.7,2.7]
    /**
    * @brief Retrieve the MDT charge value from the MDT TP
    * @return MDT charge value
    */
    uint8_t l1MdtCharge() const; // 1 bit, range [0:1]
    /**
    * @brief Retrieve the  MDT pTfrom the MDT TP
    * @return MDT pT
    */
    uint8_t l1MdtPt() const; // 8 bits, [0,127Gev] (0.5 GeV binning)
    /**
    * @brief Retrieve the  MDT eta position from the MDT TP
    * @return MDT eta position
    */
    uint16_t l1MdtEta() const; // 14 bits, range [-2.7,2.7]


    ///// setters
    /**
     * @brief Set the MDT flag
     * @param mdtFlag MDT flag
     */
    void setL1MdtFlag(uint8_t mdtFlag);
    /**
     * @brief Set the number of segments
     * @param numSegments Number of segments
     */
    void setL1NumSegments(uint8_t numSegments);
    /**
     * @brief Set the segment quality flag
     * @param flags Segment quality flag
     */
    void setL1SegmentQualityFlag(uint8_t flags);
    /**
     * @brief Set the sector logic charge
     * @param slCharge SlCharge
     */
    void setL1SlCharge(uint8_t slCharge);
    /**
     * @brief Set the sector logic pT threshold
     * @param slPtThreshold SlPtThreshold
     */
    void setL1SlPtThreshold(uint8_t slPtThreshold);
    /**
     * @brief Set the sector logic phi position
     * @param slPhiPosition SlPhiPosition
     */
    void setL1SlPhiPosition(uint16_t slPhiPosition);
    /**
    * @brief Set the sector logic eta position
    * @param slEtaPosition SlEtaPosition
    */
    void setL1SlEtaPosition(uint16_t slEtaPosition);
    /**
    * @brief Set the MDT charge value
    * @param mdtCharge MDT charge value
    */
    void setL1MdtCharge(uint8_t mdtCharge);
    /**
    * @brief Set the MDT pT value
    * @param mdtPt MDT pT value
    */
    void setL1MdtPt(uint8_t mdtPt);
    /**
    * @brief Set the MDT eta position
    * @param mdtEta MDT eta position
    */
    void setL1MdtEta(uint16_t mdtEta);

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1MDTCandData_v1, xAOD::IL1CandData_v1 );

#endif  // XAODTRIGL1MUON_VERSION_L1MDTCANDDATA_V1_H
