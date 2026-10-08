/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1MUON_VERSION_L1NSWCANDDATA_V1_H
#define XAODTRIGL1MUON_VERSION_L1NSWCANDDATA_V1_H

// System include(s):
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>

#include "AthContainers/AuxElement.h"

namespace xAOD {

  /** @brief Data class describing the L0 muon candidates from NSW-TP to TGC SL
   */

  class L1NSWCandData_v1 : public SG::AuxElement {

  public:

    L1NSWCandData_v1() = default;
    ~L1NSWCandData_v1() = default;

    /// Getters

    /**
     * @brief Retrieve the Bunch Crossing Identifier (BCID)
     * @return 12-bit bunch crossing ID
     */
    uint16_t l1Bcid() const; // 12 bits, bit order [11:0]

    /**
     * @brief Retrieve the number of segments sent from NSW-TP to Sector Logic
     * @return Number of segments
     */
    uint8_t l1NSegments() const; // 3 bits, bit order [14:12], range [0:4]

    /**
     * @brief Check if the segment candidate limit has been exceeded
     * @return True if there are more than 8 segments detected
     */
    bool l1Overflow() const; // 1 bit, bit order [15]

    /**
     * @brief Retrieve the fiber id
     * @return Fiber identifier code
     */
    uint16_t fiberID() const; // 4 bits, bit order [19:16]

    /**
     * @brief Retrieve the board id
     * @return Board identifier address
     */
    uint16_t boardID() const; // 7 bits, bit order [26:20]

    /// Segment
    /**
     * @brief Get the packed 32-bit segment word
     * @return The packed segment data word
     */
    uint32_t l1SegmentWord() const;

    /**
     * @brief Retrieve the eta index of the segment
     */
    uint16_t segEtaIndex() const;

    /**
     * @brief Retrieve the phi index of the segment
     */
    uint16_t segPhiIndex() const;

    /**
     * @brief Retrieve the delta theta index of the segment
     */
    uint8_t segDeltaThetaIndex() const;

    /**
     * @brief Retrieve the quality flag of the segment
     */
    uint8_t segQuality() const;

    /**
     * @brief Retrieve the decoded physical eta coordinate of the segment
     * @return Transformed float value representing the physical eta position
     */
    float segEta() const;

    /**
     * @brief Retrieve the decoded physical phi coordinate of the segment
     * @return Transformed float value representing the physical phi position (in radians)
     */
    float segPhi() const;

    /// Setters

    /**
     * @brief Set the Bunch Crossing Identifier (BCID)
     * @param bcid 12-bit bunch crossing ID
     */
    void setL1Bcid(uint16_t bcid);

    /**
     * @brief Set the total number of segments sent from NSW-TP to Sector Logic
     * @param nseg Segment count token
     */
    void setL1NSegments(uint8_t nseg);

    /**
     * @brief Set the segment total capacity overflow state flag
     * @param ovf Overflow flag state
     */
    void setL1Overflow(bool ovf);

    /**
     * @brief Set the fiber id
     * @param fiber Fiber id code
     */
    void setFiberID(uint16_t fiber);

    /**
     * @brief Set the board id
     * @param board Board id
     */
    void setBoardID(uint16_t board);

    /**
     * @brief Set the raw packed segment word
     * @param word Packed segment data word
     */
    void setL1SegmentWord(uint32_t word);

    /**
     * @brief Pack and set the segment from its decoded fields
     * @param etaIndex Calculated eta index
     * @param phiIndex Calculated phi index
     * @param deltaThetaIndex Calculated theta index
     * @param quality Quality flag value
     */
    void setSegment(uint16_t etaIndex, uint16_t phiIndex, uint8_t deltaThetaIndex, uint8_t quality);

    /**
     * @brief Produce a formatted string representation of the candidates
     * @return A string summary of the candidates
     */
    friend std::ostream& operator<<(std::ostream& os, const L1NSWCandData_v1& obj);

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1NSWCandData_v1, SG::AuxElement );

#endif // XAODTRIGL1MUON_VERSION_L1NSWCANDDATA_V1_H
