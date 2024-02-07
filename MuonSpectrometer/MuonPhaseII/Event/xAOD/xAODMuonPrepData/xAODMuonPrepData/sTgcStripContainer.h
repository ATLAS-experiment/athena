/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_STGCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_STGCSTRIPCONTAINER_H

#include "xAODMuonPrepData/sTgcStripCluster.h"
#include "xAODMuonPrepData/versions/sTgcStripContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Define the version of the pixel cluster container
   typedef sTgcStripContainer_v1 sTgcStripContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcStripContainer , 1114862428 , 1 )

#endif