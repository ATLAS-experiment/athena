/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCWIREHIT_V1_H
#define XAODMUONPREPDATA_VERSION_STGCWIREHIT_V1_H

#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"

namespace xAOD {

class sTgcWireHit_v1 : public sTgcMeasurement_v1 {

  public:
    /// Default constructor
    sTgcWireHit_v1() = default;
    /// Virtual destructor
    virtual ~sTgcWireHit_v1() = default;
    /// Returns the type of the Tgc strip as a simple enumeration
    sTgcChannelTypes channelType() const override final {
        return sTgcChannelTypes::Wire;
    }
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE_FIN(xAOD::sTgcWireHit_v1, xAOD::sTgcMeasurement_v1);

#endif