/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCSTRIPCLUSTER_H
#define XAODMUONPREPDATA_STGCSTRIPCLUSTER_H

#include "xAODMuonPrepData/sTgcStripClusterFwd.h"
#include "xAODMuonPrepData/versions/sTgcStripCluster_v1.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
DATAVECTOR_BASE(xAOD::sTgcStripCluster_v1, xAOD::sTgcMeasurement_v1);


// Set up a CLID for the class:

CLASS_DEF( xAOD::sTgcStripCluster , 213072997 , 1 )

#endif