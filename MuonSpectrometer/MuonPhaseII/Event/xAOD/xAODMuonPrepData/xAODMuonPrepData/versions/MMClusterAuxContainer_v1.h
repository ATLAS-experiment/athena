/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_MMCLUSTERAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_MMCLUSTERAUXCONTAINER_V1_H

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class MMClusterAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    MMClusterAuxContainer_v1();

   private:
    /// @name Defining Mdt Drift Circle parameters
    /// @{
    std::vector<DetectorIdentType> identifier{};
    std::vector<DetectorIDHashType> identifierHash{};
    std::vector<PosAccessor<1>::element_type> localPosition{};
    std::vector<CovAccessor<1>::element_type> localCovariance{};
    
    std::vector<uint8_t>  gasGap{};
    std::vector<uint16_t> channelNumber{};
  
    std::vector<uint16_t> time{};
    std::vector<uint32_t> charge{}; 
    std::vector<float>    driftDist{};
    std::vector<float>    angle{};
    std::vector<float>    chiSqProb{};
    std::vector<short>    author{};
    std::vector<uint8_t>  quality{};

    std::vector<std::vector<uint16_t>>  stripNumbers{};
    std::vector<std::vector<int16_t>>   stripTimes{};
    std::vector<std::vector<int>>       stripCharges{};
    std::vector<std::vector<float>>     stripDriftDist{};
    std::vector<std::vector<PosAccessor<2>::element_type>> stripDriftErrors{};
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::MMClusterAuxContainer_v1, xAOD::AuxContainerBase);
#endif
