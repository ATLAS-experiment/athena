/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCWIREAUXCONTAINER_H
#define XAODMUONPREPDATA_STGCWIREAUXCONTAINER_H

#include "xAODMuonPrepData/versions/sTgcWireAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcWire
   typedef sTgcWireAuxContainer_v1 sTgcWireAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcWireAuxContainer , 1093340389 , 1 )
#endif  // XAODMUONPREPDATA_STGCSTRIPAUXCONTAINER_H