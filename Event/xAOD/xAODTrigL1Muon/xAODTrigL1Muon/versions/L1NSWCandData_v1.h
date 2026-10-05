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

    /// Segments
    /**
     * @brief Get the complete collection of packed 32-bit segment words
     * @return Reference to the vector of packed segment data words
     */
    const std::vector<uint32_t>& l1SegmentWords() const;

    /// Element-by-Element Index
    /**
     * @brief Retrieve the eta index for a specific segment
     * @param i Index of the segment within the vector
     */
    uint16_t segEtaIndex(size_t i) const;

    /**
     * @brief Retrieve the phi index for a specific segment
     * @param i Index of the segment within the vector
     */
    uint16_t segPhiIndex(size_t i) const;

    /**
     * @brief Retrieve the delta theta index for a specific segment
     * @param i Index of the segment within the vector
     */
    uint8_t segDeltaThetaIndex(size_t i) const;

    /**
     * @brief Retrieve the quality flag for a specific segment
     * @param i Index of the segment within the vector
     */
    uint8_t segQuality(size_t i) const;

    /**
     * @brief Retrieve the decoded physical eta coordinate for a specific segment
     * @param i Index of the segment within the vector
     * @return Transformed float value representing the physical eta position
     */
    float segEta(size_t i) const;

    /**
     * @brief Retrieve the decoded physical phi coordinate for a specific segment
     * @param i Index of the segment within the vector
     * @return Transformed float value representing the physical phi position (in radians)
     */
    float segPhi(size_t i) const;

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
     * @brief Pack and append a single segment
     * @brief Push a new completed segment into the candidate's segment collections
     * @param etaIndex Calculated eta index
     * @param phiIndex Calculated phi index
     * @param deltaThetaIndex Calculated theta index
     * @param quality Quality flag value
     */
    void addSegment(uint16_t etaIndex, uint16_t phiIndex, uint8_t deltaThetaIndex, uint8_t quality);

    /**
     * @brief Clear all recorded segment vectors
     */
    void clearSegments();

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
