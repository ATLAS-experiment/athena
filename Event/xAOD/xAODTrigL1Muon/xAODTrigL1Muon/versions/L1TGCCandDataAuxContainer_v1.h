/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1MUON_VERSION_L1TGCCANDDATAUXCONTAINER_V1_H
#define XAODTRIGL1MUON_VERSION_L1TGCCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODTrigL1Muon/versions/L1TGCCandData_v1.h"

namespace xAOD {
    /// Auxiliary store for TGC candidate data
    ///
    class L1TGCCandDataAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        L1TGCCandDataAuxContainer_v1();

    private:
        std::vector<uint16_t> l1Eta {};
        std::vector<uint16_t> l1Phi {};
        std::vector<uint8_t> l1Pt {};
        std::vector<uint8_t> l1Threshold {};
        std::vector<uint8_t> l1CandCharge {};
        std::vector<uint16_t> l1SubdetectorId {};
        std::vector<uint16_t> l1SectorId {};
        std::vector<uint16_t> bcTag {};
        std::vector<uint8_t> coinType {};
        std::vector<uint8_t> l1DeltaPhiWord {};
        std::vector<uint8_t> l1DeltaThetaWord {};
        std::vector<uint32_t> l1NswSegment {};
        std::vector<uint8_t> tcId {};

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::L1TGCCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1MUON_VERSION_L1TGCCANDDATAUXCONTAINER_V1_H
