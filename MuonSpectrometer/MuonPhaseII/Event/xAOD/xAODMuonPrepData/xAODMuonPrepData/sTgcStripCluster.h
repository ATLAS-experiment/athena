/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCSTRIPCLUSTER_H
#define XAODMUONPREPDATA_STGCSTRIPCLUSTER_H

#include "xAODMuonPrepData/versions/sTgcStripCluster_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
   /// Defined the version of the sTgcStrip
   typedef sTgcStripCluster_v1 sTgcStripCluster;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::sTgcStripCluster , 213072997 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
