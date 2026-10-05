/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_VERSION_L1MDTCANDDATAUXCONTAINER_V1_H
#define XAODTRIGL1MUON_VERSION_L1MDTCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODTrigL1Muon/versions/L1MDTCandData_v1.h"

namespace xAOD {
    /// Auxiliary store for MDT candidate data
    ///
    class L1MDTCandDataAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        L1MDTCandDataAuxContainer_v1();

    private:
        // RPC/TGC candidate data (from IL1CandData interface)
        std::vector<uint16_t> l1Eta {};
        std::vector<uint16_t> l1Phi {};
        std::vector<uint8_t> l1Pt {};
        std::vector<uint8_t> l1Threshold {};
        std::vector<uint8_t> l1CandCharge {};
        std::vector<uint16_t> l1SubdetectorId {};
        std::vector<uint16_t> l1SectorId {};
        std::vector<uint16_t> bcTag {};
        std::vector<uint8_t> coinType {};
        std::vector<uint8_t> tcId {};

        // MDT-only data (moved out of the shared interface)
        std::vector<uint8_t> l1MdtFlag {};

        // MDT-specific data
        std::vector<uint8_t> l1NumSegments {};
        std::vector<uint8_t> l1SegmentQualityFlag {};
        std::vector<uint8_t> l1SlCharge {};
        std::vector<uint8_t> l1SlPtThreshold {};
        std::vector<uint16_t> l1SlPhiPosition {};
        std::vector<uint16_t> l1SlEtaPosition {};
        std::vector<uint8_t> l1MdtCharge {};
        std::vector<uint8_t> l1MdtPt {};
        std::vector<uint16_t> l1MdtEta {};

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1MDTCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1MUON_VERSION_L1MDTCANDDATAUXCONTAINER_V1_H
