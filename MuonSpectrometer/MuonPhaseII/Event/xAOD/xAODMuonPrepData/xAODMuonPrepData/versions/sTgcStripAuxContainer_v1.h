/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_STGCSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_STGCSTRIPAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class sTgcStripAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    sTgcStripAuxContainer_v1();

   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<1>::element_type> localPosition{};
    std::vector<CovAccessor<1>::element_type> localCovariance{};

    std::vector<uint16_t> bcBitMap{};
    std::vector<uint16_t> time{};
    std::vector<uint32_t> charge{}; 
 
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::sTgcStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif
