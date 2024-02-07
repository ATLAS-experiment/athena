/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCPADHIT_V1_H
#define XAODMUONPREPDATA_VERSION_STGCPADHIT_V1_H

#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"

namespace xAOD {

class sTgcPadHit_v1 : public sTgcMeasurement_v1 {

  public:
    /// Default constructor
    sTgcPadHit_v1() = default;
    /// Virtual destructor
    virtual ~sTgcPadHit_v1() = default;

    /// Returns the type of the Tgc strip as a simple enumeration
    sTgcChannelTypes channelType() const override final {
        return sTgcChannelTypes::Pad;
    }
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE_FIN(xAOD::sTgcPadHit_v1, xAOD::sTgcMeasurement_v1);

#endif