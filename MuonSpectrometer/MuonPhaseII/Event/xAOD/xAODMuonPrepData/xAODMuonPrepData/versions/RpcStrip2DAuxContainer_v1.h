/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_BIRPCSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_BIRPCSTRIPAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class RpcStrip2DAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    RpcStrip2DAuxContainer_v1();

   private:
    /// @name Defining Rpc strip parameter
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<2>::element_type> localPosition{};
    std::vector<CovAccessor<2>::element_type> localCovariance{};

    std::vector<float> time{};
    std::vector<float> timeCovariance{};
    std::vector<uint32_t> triggerInfo{}; // FIXME - how big do we need this to be?
    std::vector<uint8_t> ambiguityFlag{};
    std::vector<float> timeOverThreshold{};

    std::vector<uint16_t> channelNumber{};
    std::vector<uint8_t> gasGap{};
    std::vector<uint8_t> doubletPhi{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::RpcStrip2DAuxContainer_v1, xAOD::AuxContainerBase);
#endif
