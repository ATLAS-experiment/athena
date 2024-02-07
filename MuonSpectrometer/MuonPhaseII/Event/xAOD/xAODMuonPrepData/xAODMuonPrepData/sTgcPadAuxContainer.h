/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCPADAUXCONTAINER_H
#define XAODMUONPREPDATA_STGCPADAUXCONTAINER_H

#include "xAODMuonPrepData/versions/sTgcPadAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcWire
   typedef sTgcPadAuxContainer_v1 sTgcPadAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcPadAuxContainer , 1246485600 , 1 )
#endif  // XAODMUONPREPDATA_STGCSTRIPAUXCONTAINER_H