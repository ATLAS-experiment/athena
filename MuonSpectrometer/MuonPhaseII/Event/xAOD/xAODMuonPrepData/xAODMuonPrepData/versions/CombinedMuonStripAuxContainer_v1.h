/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_COMBINEDMUONSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_COMBINEDMUONSTRIPAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/ElementLink.h"

namespace xAOD {
/// Auxiliary store for Mdt drift circles
///
class CombinedMuonStripAuxContainer_v1 : public AuxContainerBase {
        public:
            /** @brief Empty constructor */
            CombinedMuonStripAuxContainer_v1();
            /** @brief virtual destructor */
            virtual ~CombinedMuonStripAuxContainer_v1() = default;
        private:
            using Link_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
            std::vector<Link_t> MuonStripLink1{};
            std::vector<Link_t> MuonStripLink2{};
            

    };
}
// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::CombinedMuonStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif