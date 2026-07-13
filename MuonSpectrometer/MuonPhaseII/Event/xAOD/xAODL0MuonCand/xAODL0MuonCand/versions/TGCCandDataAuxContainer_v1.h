/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODL0MUONCAND_VERSION_TGCCANDDATAUXCONTAINER_V1_H
#define XAODL0MUONCAND_VERSION_TGCCANDDATAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "xAODL0MuonCand/versions/TGCCandData_v1.h"

namespace xAOD {
    /// Auxiliary store for TGC candidate data
    ///
    class TGCCandDataAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        TGCCandDataAuxContainer_v1();

    private:
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
        std::vector<float> deltaPhi {};
        std::vector<float> deltaTheta {};
        std::vector<uint32_t> nswSegment {};
        std::vector<xAOD::ICandData_v1::Quality> candQuality {};
        std::vector<uint8_t> tcId {};
        std::vector<uint8_t> passedPtThresholdIndex {};
        std::vector<uint8_t> estimatedPtValueIndex {};
        std::vector<uint8_t> estimatedPtValueValid {};
        std::vector<uint8_t> side {};
        std::vector<uint8_t> endcapFlag {};
        std::vector<uint8_t> sector {};

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::TGCCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODL0MUONCAND_VERSION_TGCCANDDATAUXCONTAINER_V1_H
