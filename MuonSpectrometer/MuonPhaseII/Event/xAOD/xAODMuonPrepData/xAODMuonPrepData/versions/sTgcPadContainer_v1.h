/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCPADCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSION_STGCPADCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/versions/sTgcPadHit_v1.h"

namespace xAOD {
   /// The container is a simple typedef for now
   typedef DataVector<xAOD::sTgcPadHit_v1> sTgcPadContainer_v1;
}  // namespace xAOD

#endif