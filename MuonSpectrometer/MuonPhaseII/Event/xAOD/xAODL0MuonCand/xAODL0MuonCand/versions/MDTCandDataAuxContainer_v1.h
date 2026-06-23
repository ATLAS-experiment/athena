/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODL0MUONCAND_VERSION_MDTCANDDATAUXCONTAINER_V1_H
#define XAODL0MUONCAND_VERSION_MDTCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODL0MuonCand/versions/MDTCandData_v1.h"

namespace xAOD {
    /// Auxiliary store for MDT candidate data
    ///
    class MDTCandDataAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        MDTCandDataAuxContainer_v1();

    private:
        // RPC/TGC candidate data (from ICandData interface)
        std::vector<uint16_t> eta {};
        std::vector<uint16_t> phi {};
        std::vector<uint8_t> pt {};
        std::vector<uint8_t> threshold {};
        std::vector<uint8_t> candCharge {};
        std::vector<uint8_t> mdtFlag {};
        std::vector<uint16_t> subdetectorId {};
        std::vector<uint16_t> sectorId {};
        std::vector<uint16_t> bcTag {};
        std::vector<uint8_t> coinType {};

        // MDT-specific data
        std::vector<uint8_t> numSegments {};
        std::vector<uint8_t> segmentQualityFlag {};
        std::vector<uint8_t> slCharge {};
        std::vector<uint8_t> slPtThreshold {};
        std::vector<uint8_t> slPhiPosition {};
        std::vector<uint8_t> tcIdentifier {};

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::MDTCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODL0MUONCAND_VERSION_MDTCANDDATAUXCONTAINER_V1_H
