/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODL0MUONCAND_VERSION_NSWCANDDATAUXCONTAINER_V1_H
#define XAODL0MUONCAND_VERSION_NSWCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODL0MuonCand/versions/NSWCandData_v1.h"

namespace xAOD {

  /// Auxiliary store for NSW candidate data
  class NSWCandDataAuxContainer_v1 : public AuxContainerBase {
  public:
    /// Default constructor
    NSWCandDataAuxContainer_v1();

  private:

    /// Bunch crossing ID (12 bits, bit order [11:0])
    std::vector<uint16_t> bcid {};
    /// Number of segments from NSW to SL (3 bits, bit order [14:12])
    std::vector<uint8_t>  nSegments {};
    /// Overflow flag in case that there are more segments than 8 (1 bit, bit order [15]) 
    std::vector<uint8_t>  overflow {};
    /// Fiber ID (4 bits, bit order [19:16])
    std::vector<uint8_t>  fiberId {};
    /// Board ID (7 bits, bit order [26:20])
    std::vector<uint8_t>  boardId {};
    /// The attributes of each segment are packed in a 32-bit word.
    /// eta, phi, deltaTheta, quality
    std::vector<uint32_t> segmentWord {}; 
    
  };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::NSWCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODL0MUONCAND_VERSION_NSWCANDDATAUXCONTAINER_V1_H
