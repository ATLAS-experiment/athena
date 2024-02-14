/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_RPCSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_RPCSTRIPAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class RpcStripAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    RpcStripAuxContainer_v1();

   private:
    /// @name Defining Rpc strip parameter
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<1>::element_type> localPosition{};
    std::vector<CovAccessor<1>::element_type> localCovariance{};

    std::vector<float> time{};
    std::vector<uint32_t> triggerInfo{}; // FIXME - how big do we need this to be?
    std::vector<uint8_t> ambiguityFlag{};
    std::vector<float> timeOverThreshold{};
    std::vector<PosAccessor<3>::element_type> stripPosInStation{};

    std::vector<uint16_t> stripNumber{};
    std::vector<uint8_t> gasGap{};
    std::vector<uint8_t> doubletPhi{};
    std::vector<uint8_t> measPhi{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::RpcStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif
