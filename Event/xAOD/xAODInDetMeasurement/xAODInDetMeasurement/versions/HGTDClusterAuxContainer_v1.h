/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_V1_H
#define XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_V1_H

#include <vector>

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"


namespace xAOD {
    /// Auxiliary store for HGTD clusters
    class HGTDClusterAuxContainer_v1 : public AuxContainerBase {
        public:
            /// Default constructor
            HGTDClusterAuxContainer_v1();

        private:
        /// @name Defining uncalibrated measurement parameters
            /// @{
            std::vector<DetectorIdentType> identifier;
            std::vector<DetectorIDHashType> identifierHash;
            std::vector<PosAccessor<3>::element_type> localPosition;
            std::vector<CovAccessor<3>::element_type> localCovariance;
            /// @}

        /// @name Defining HGTD cluster parameters
        /// @{
        std::vector<std::vector<Identifier::value_type> > rdoList;
        std::vector<std::vector<int> > totList;

    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::HGTDClusterAuxContainer_v1, xAOD::AuxContainerBase);


#endif // XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_V1_H

