/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MMCLUSTER_H
#define XAODMUONPREPDATA_MMCLUSTER_H

#include "xAODMuonPrepData/MMClusterFwd.h"
#include "xAODMuonPrepData/versions/MMCluster_v1.h"
#include "AthContainers/DataVector.h"
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"

DATAVECTOR_BASE(xAOD::MMCluster_v1, xAOD::UncalibratedMeasurement_v1);

CLASS_DEF(xAOD::MMCluster, 95201827, 1)

#endif