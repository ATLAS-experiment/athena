/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_VERSIONS_COMBINEDMUONSTRIPAUXCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSIONS_COMBINEDMUONSTRIPAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"
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

            using Link_t = ElementLink<xAOD::MuonMeasurementContainer>;
            std::vector<Link_t> MuonStripLink1{};
            std::vector<Link_t> MuonStripLink2{};
            std::vector<PosAccessor<2>::element_type> localPosition{};
            std::vector<CovAccessor<2>::element_type> localCovariance{};

    };
}
// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::CombinedMuonStripAuxContainer_v1, xAOD::AuxContainerBase);
#endif