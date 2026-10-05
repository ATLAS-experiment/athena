/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_VERSION_L1NSWCANDDATAUXCONTAINER_V1_H
#define XAODTRIGL1MUON_VERSION_L1NSWCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODTrigL1Muon/versions/L1NSWCandData_v1.h"

namespace xAOD {

  /// Auxiliary store for NSW candidate data
  class L1NSWCandDataAuxContainer_v1 : public AuxContainerBase {
  public:
    /// Default constructor
    L1NSWCandDataAuxContainer_v1();

  private:

    /// Bunch crossing ID (12 bits, bit order [11:0])
    std::vector<uint16_t> l1Bcid {};
    /// Number of segments from NSW to SL (3 bits, bit order [14:12])
    std::vector<uint8_t>  l1NSegments {};
    /// Overflow flag in case that there are more segments than 8 (1 bit, bit order [15])
    std::vector<uint8_t>  l1Overflow {};
    /// Fiber ID (4 bits, bit order [19:16]) -- aligned with SectorLogicCandData's own fiberID
    std::vector<uint16_t>  fiberID {};
    /// Board ID (7 bits, bit order [26:20]) -- aligned with SectorLogicCandData's own boardID
    std::vector<uint16_t>  boardID {};
    /// The attributes of each segment are packed in a 32-bit word.
    /// eta, phi, deltaTheta, quality
    std::vector<uint32_t> l1SegmentWords {};

  };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1NSWCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1MUON_VERSION_L1NSWCANDDATAUXCONTAINER_V1_H
