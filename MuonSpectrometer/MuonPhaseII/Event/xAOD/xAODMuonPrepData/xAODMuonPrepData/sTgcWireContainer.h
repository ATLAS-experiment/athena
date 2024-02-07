/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCWIRECONTAINER_H
#define XAODMUONPREPDATA_STGCWIRECONTAINER_H

#include "xAODMuonPrepData/sTgcWireHit.h"
#include "xAODMuonPrepData/versions/sTgcWireContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcStrip
   typedef sTgcWireContainer_v1 sTgcWireContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcWireContainer , 1281983002 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
