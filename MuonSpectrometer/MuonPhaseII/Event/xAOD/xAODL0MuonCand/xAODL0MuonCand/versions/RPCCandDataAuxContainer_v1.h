/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODL0MUONCAND_VERSION_RPCCANDDATAUXCONTAINER_V1_H
#define XAODL0MUONCAND_VERSION_RPCCANDDATAUXCONTAINER_V1_H


#include <array>
#include <cstdint>

#include "xAODCore/AuxContainerBase.h"

#include "xAODL0MuonCand/versions/RPCCandData_v1.h"

namespace xAOD {
    /// Auxiliary store for pixel clusters
    ///
    class RPCCandDataAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        RPCCandDataAuxContainer_v1();

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
        std::vector<std::array<uint16_t, 4>> zPos {};
        std::vector<xAOD::ICandData_v1::Quality> candQuality {};

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::RPCCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODL0MUONCAND_VERSION_RPCCANDDATAUXCONTAINER_V1_H

