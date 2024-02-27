/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSIONS_STGCWIREAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_STGCWIREAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class sTgcWireAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    sTgcWireAuxContainer_v1();

   private:
    /// @name Defining sTgcStrip parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<1>::element_type> localPosition{};
    std::vector<CovAccessor<1>::element_type> localCovariance{};
    /// 
    std::vector<uint8_t> author{};
    std::vector<uint8_t> gasGap{};
    std::vector<uint16_t> channelNumber{};
    std::vector<short int> time{};
    std::vector<int> charge{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::sTgcWireAuxContainer_v1, xAOD::AuxContainerBase);
#endif
