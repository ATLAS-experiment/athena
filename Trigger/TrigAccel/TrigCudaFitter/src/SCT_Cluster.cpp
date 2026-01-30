// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "Surface.h"
#include "SCT_Cluster.h"

SCT_Cluster::SCT_Cluster(std::unique_ptr<const Surface> pS) : SiCluster(std::move(pS))
{
}
