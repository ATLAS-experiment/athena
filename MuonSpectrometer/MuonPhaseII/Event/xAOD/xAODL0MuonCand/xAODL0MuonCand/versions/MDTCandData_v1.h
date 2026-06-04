/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODL0MUONCAND_VERSION_MDTCANDDATA_V1_H
#define XAODL0MUONCAND_VERSION_MDTCANDDATA_V1_H

// System include(s):
#include <cstdint>

#include "AthContainers/AuxElement.h"
#include "xAODL0MuonCand/ICandData.h"

namespace xAOD
{

    /** @brief Data class describing the L0 muon candidates from MDT-TP to TGC/RPC SL
  */

  class MDTCandData_v1 : public ICandData_v1 {
  public:
    // default constructor and destructor
    MDTCandData_v1() = default;
    ~MDTCandData_v1() = default;

    /// Quality flag for each segment
    enum class SegmentQuality : uint8_t
    {
      Q_UNDEFINED = 0,
      Q_BEST,
      Q_LOW
    };

    ///// getters
    /**
     * @brief Retrieve the number of MDT segments asspcoate to the muon candidate
     * @return Number of segments
     */
    uint8_t numSegments() const; // 2 bits, range [0:3]
    /**
     * @brief Retrieve the segment quality flag for each segment
     * @return Segment quality flag
     */
    uint8_t segmentQualityFlag() const;  //3 bits, segment quality flags ( [0:1] per segment)
    /**
     * @brief Retrieve the charge value from the sector logic
     * @return sector logic charge
     */
    uint8_t slCharge() const; // 1 bit, range [0:1]
    /**
     * @brief Retrieve the highest RPC/TGC pT threshold satisfied in sector logic
     * @return sector logic pT threshold
     */
    uint8_t slPtThreshold() const; // 4 bits, highest RPC/TGC pT threshold satisfied (15 levels)
    /**
     * @brief Retrieve the phi position in global coordinates from the sector logic
     * @return sector logic phi position
     */
    uint16_t slPhiPosition() const; // 9 bits, range [0,2PI]
    /**
     * @brief Retrieve the eta position in global coordinates from the sector logic
     * @return sector logic eta position
     */
    uint16_t slEtaPosition() const; // 14 bits, range [-2.7,2.7]
    /**
     * @brief Retrieve the trigger candidate identifier
     * @return trigger candidate identifier
     */
    uint8_t tcIdentifier() const; // 3 bits, range [0:6]
    /**
    * @brief Retrieve the MDT charge value from the MDT TP
    * @return MDT charge value
    */
    uint8_t MDTCarge() const; // 1 bit, range [0:1]
    /**
    * @brief Retrieve the  MDT pTfrom the MDT TP
    * @return MDT pT 
    */
    uint8_t MDTPt() const; // 8 bits, [0,127Gev] (0.5 GeV binning)
    /**
    * @brief Retrieve the  MDT eta position from the MDT TP
    * @return MDT eta position
    */
    uint16_t MDEta() const; // 14 bits, range [-2.7,2.7]


    ///// setters 
    /**
     * @brief Set the number of segments
     * @param numSegments Number of segments
     */
    void setNumSegments(uint8_t numSegments);
    /**
     * @brief Set the segment quality flag
     * @param flags Segment quality flag
     */
    void setSegmentQualityFlag(uint8_t flags);
    /**
     * @brief Set the sector logic charge
     * @param slCharge SlCharge
     */
    void setSlCharge(uint8_t slCharge);
    /**
     * @brief Set the sector logic pT threshold
     * @param slPtThreshold SlPtThreshold
     */
    void setSlPtThreshold(uint8_t slPtThreshold);
    /**
     * @brief Set the sector logic phi position
     * @param slPhiPosition SlPhiPosition
     */
    void setSlPhiPosition(uint16_t slPhiPosition);
    /**
    * @brief Set the sector logic eta position
    * @param slEtaPosition SlEtaPosition
    */
    void setSlEtaPosition(uint16_t slEtaPosition);
    /**
     * @brief Set the trigger candidate identifier
     * @param tcIdentifier TcIdentifier
     */
    void setTcIdentifier(uint8_t tcIdentifier);
    /**
    * @brief Set the MDT charge value
    * @param mdtCharge MDT charge value
    */
    void setMDTCharge(uint8_t mdtCharge);
    /**
    * @brief Set the MDT pT value
    * @param mdtPt MDT pT value
    */
    void setMDTPt(uint8_t mdtPt);
    /**
    * @brief Set the MDT eta position
    * @param mdtEta MDT eta position
    */
    void setMDTEta(uint16_t mdtEta);

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::MDTCandData_v1, SG::AuxElement );

#endif  // XAODL0MUONCAND_VERSION_MDTCANDDATA_V1_H
