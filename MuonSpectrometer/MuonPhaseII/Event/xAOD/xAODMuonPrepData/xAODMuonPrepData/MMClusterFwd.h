/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MMClusterFWD_H
#define XAODMUONPREPDATA_MMClusterFWD_H

/** @brief Forward declaration of the xAOD::MMCluster */
namespace xAOD{
   class UncalibratedMeasurement_v1;
   class MMCluster_v1;
   using MMCluster = MMCluster_v1;

   class MMClusterAuxContainer_v1;
   using MMClusterAuxContainer = MMClusterAuxContainer_v1;
}
#endif
