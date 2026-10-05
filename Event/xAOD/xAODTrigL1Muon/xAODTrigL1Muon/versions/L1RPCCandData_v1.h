/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1MUON_VERSION_L1RPCCANDDATA_V1_H
#define XAODTRIGL1MUON_VERSION_L1RPCCANDDATA_V1_H

// System include(s):
#include <cstdint>
#include <array>
#include "AthContainers/AuxElement.h"
#include "xAODTrigL1Muon/IL1CandData.h"

namespace xAOD {

    /** @brief Data class describing the L0 muon candidates from RPC-SL to MDT-TP
  */

  class L1RPCCandData_v1 : public IL1CandData_v1 {
  public:
    // default constructor and destructor
    L1RPCCandData_v1() = default;
    ~L1RPCCandData_v1() = default;

    /**
     * @brief Retrieve the global z positions of the RPC sector logic
     * @return Array of z positions for the RPC sector logic
     */
    std::array<uint16_t, 4> l1ZPos() const; // 10 bits each, range [0:16.368] (16 mm binning)
    /**
     * @brief Set the z positions of the RPC sector logic
     * @param zPos Array of z positions for the RPC sector logic
    */
    void setL1ZPos(std::array<float, 4>& zPos);

    static constexpr uint16_t zPosBitRange() { return s_zPosBitRange; }
    static constexpr float zPosRange() { return s_zPosRange; }

  private:
    /// range of the RPC hits z positions
    static constexpr float s_zPosRange = 12500.0F;
    /// 12 bits for z position
    static constexpr uint16_t s_zPosBitRange = 0x0fff;

  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1RPCCandData_v1, xAOD::IL1CandData_v1 );

#endif  // XAODTRIGL1MUON_VERSION_L1RPCCANDDATA_V1_H
