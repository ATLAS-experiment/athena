/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCWIRECONTAINER_H
#define XAODMUONPREPDATA_VERSION_STGCWIRECONTAINER_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/versions/sTgcWireHit_v1.h"

namespace xAOD {
   /// The container is a simple typedef for now
   typedef DataVector<xAOD::sTgcWireHit_v1> sTgcWireContainer_v1;
}  // namespace xAOD

#endif