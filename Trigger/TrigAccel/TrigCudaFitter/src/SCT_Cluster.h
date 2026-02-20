// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __SCT_CLUSTER_H__
#define __SCT_CLUSTER_H__

#include "SiCluster.h"

#include <memory>

class SCT_Cluster : public SiCluster
{
  public:
    SCT_Cluster(std::unique_ptr<const Surface>);
  
    virtual void setParameters(float* par) = 0;
    virtual TrkBaseNode* createDkfNode(void) const = 0;

    double m_m{};
    double m_cov{};
};

#endif
