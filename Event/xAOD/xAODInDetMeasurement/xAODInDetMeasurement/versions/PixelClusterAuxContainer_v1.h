/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSIONS_PIXELCLUSTERAUXCONTAINER_V1_H
#define XAODINDETMEASUREMENT_VERSIONS_PIXELCLUSTERAUXCONTAINER_V1_H

#include <vector>

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD {
/// Auxiliary store for pixel clusters
///
class PixelClusterAuxContainer_v1 : public AuxContainerBase {
   public:
    /// Default constructor
    PixelClusterAuxContainer_v1();

   private:
    /// @name Defining uncalibrated measurement parameters
    /// @{
    std::vector<DetectorIdentType> identifier;
    std::vector<DetectorIDHashType> identifierHash;
    std::vector<PosAccessor<2>::element_type> localPosition;
    std::vector<CovAccessor<2>::element_type> localCovariance;
    /// @}

    /// @name Defining pixel cluster parameters
    /// @{
    std::vector<PosAccessor<3>::element_type> globalPosition;
    std::vector<std::vector<Identifier::value_type> > rdoList;
    std::vector<int> channelsInPhi;
    std::vector<int> channelsInEta;
    std::vector<float> widthInEta;
    std::vector<std::vector<int> > totList;
    std::vector<int> totalToT;
    std::vector<std::vector<float> > chargeList;
    std::vector<float> totalCharge;
    std::vector<float> energyLoss;
    std::vector<char> isSplit;
    std::vector<float> splitProbability1;
    std::vector<float> splitProbability2;
    std::vector<int> lvl1a;
    /// @}
};
}  // namespace xAOD

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::PixelClusterAuxContainer_v1, xAOD::AuxContainerBase);

#endif  // XAODINDETMEASUREMENT_VERSIONS_PIXELCLUSTERAUXCONTAINER_V1_H
