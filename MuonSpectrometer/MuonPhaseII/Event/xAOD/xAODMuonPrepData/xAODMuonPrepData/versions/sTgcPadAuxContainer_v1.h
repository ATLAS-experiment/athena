/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_STGCPADAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_STGCPADAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class sTgcPadAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    sTgcPadAuxContainer_v1();

   private:
    /// @name Defining sTgcStrip parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<2>::element_type> localPosition{};
    std::vector<CovAccessor<2>::element_type> localCovariance{};
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
SG_BASE(xAOD::sTgcPadAuxContainer_v1, xAOD::AuxContainerBase);
#endif
