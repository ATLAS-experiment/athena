/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_TGCSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_TGCSTRIPAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class TgcStripAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    TgcStripAuxContainer_v1();

   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<1>::element_type> localPosition{};
    std::vector<CovAccessor<1>::element_type> localCovariance{};

    std::vector<uint16_t> bcBitMap{};
    
    std::vector<uint16_t> channelNumber{};
    std::vector<uint8_t> gasGap{};
    std::vector<uint8_t> measuresPhi{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TgcStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif
