/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_sTgcStripClusterFWD_H
#define XAODMUONPREPDATA_sTgcStripClusterFWD_H
#include "xAODMuonPrepData/sTgcMeasurementFwd.h"
/** @brief Forward declaration of the xAOD::sTgcStripCluster */
namespace xAOD{
   class sTgcMeasurement_v1;
   class sTgcStripCluster_v1;
   using sTgcStripCluster = sTgcStripCluster_v1;

   class sTgcStripAuxContainer_v1;
   using sTgcStripAuxContainer = sTgcStripAuxContainer_v1;
}
#endif
