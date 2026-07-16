/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSIONS_STRIPCLUSTERAUXCONTAINER_V1_H
#define XAODINDETMEASUREMENT_VERSIONS_STRIPCLUSTERAUXCONTAINER_V1_H

#include <vector>

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/JaggedVec.h"
#include "AthContainers/JaggedVecAccessor.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODInDetMeasurement/ArrayFloat3.h"

namespace xAOD {
/// Auxiliary store for strip clusters
///
class StripClusterAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    StripClusterAuxContainer_v1();

   private:
    /// @name Defining uncalibrated measurement parameters
    /// @{
    std::vector<DetectorIdentType> identifier;
    std::vector<DetectorIDHashType> identifierHash;
    std::vector<PosAccessor<1>::element_type> localPosition;
    std::vector<CovAccessor<1>::element_type> localCovariance;
    /// @}

    /// @name Defining strip cluster parameters
    /// @{
    std::vector<xAOD::ArrayFloat3> globalPosition;
    AUXVAR_JAGGEDVEC_DECL(Identifier::value_type,rdoList);
    std::vector<int> channelsInPhi;
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::StripClusterAuxContainer_v1, xAOD::AuxContainerBase);

#endif  // XAODINDETMEASUREMENT_VERSIONS_STRIPCLUSTERAUXCONTAINER_V1_H
