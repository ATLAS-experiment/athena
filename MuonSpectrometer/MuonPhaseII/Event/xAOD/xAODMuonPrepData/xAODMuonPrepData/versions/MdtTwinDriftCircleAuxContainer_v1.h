/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_MDTTWINDRIFTCIRCLEAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_MDTTWINDRIFTCIRCLEAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class MdtTwinDriftCircleAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    MdtTwinDriftCircleAuxContainer_v1();

   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<2>::element_type> localPosition{};
    std::vector<CovAccessor<2>::element_type> localCovariance{};

    std::vector<int16_t> tdc{};
    std::vector<int16_t> adc{};
    std::vector<uint16_t> driftTube{};
    std::vector<uint8_t> tubeLayer{};
    std::vector<uint8_t> status{};

    std::vector<int16_t> twinTdc{};
    std::vector<int16_t> twinAdc{};
    std::vector<uint16_t> twinTube{};
    std::vector<uint8_t> twinLayer{};

    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::MdtTwinDriftCircleAuxContainer_v1, xAOD::AuxContainerBase);
#endif
