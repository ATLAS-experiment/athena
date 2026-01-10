// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __SCT_BARREL_CLUSTER_H__
#define __SCT_BARREL_CLUSTER_H__

#include "SCT_Cluster.h"

#include <memory>

class SCT_BarrelCluster : public SCT_Cluster
{
  public:
    SCT_BarrelCluster(std::unique_ptr<const Surface>);
  
    virtual void setParameters(float* par);
    virtual TrkBaseNode* createDkfNode(void) const;
};
#endif
