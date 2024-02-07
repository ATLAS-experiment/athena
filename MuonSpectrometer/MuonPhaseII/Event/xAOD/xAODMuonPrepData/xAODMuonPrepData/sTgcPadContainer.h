/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_STGCPADCONTAINER_H
#define XAODMUONPREPDATA_STGCPADCONTAINER_H

#include "xAODMuonPrepData/sTgcPadHit.h"
#include "xAODMuonPrepData/versions/sTgcPadContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Define the version of the pixel cluster container
   typedef sTgcPadContainer_v1 sTgcPadContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcPadContainer , 1310723933 , 1 )
#endif