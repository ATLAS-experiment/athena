// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __SI_CLUSTER_H__
#define __SI_CLUSTER_H__

#include "Surface.h"

#include <memory>

class TrkBaseNode;

class SiCluster
{
  public:
    SiCluster(std::unique_ptr<const Surface>);
    virtual ~SiCluster(void) = default;

    virtual void setParameters(float* par) = 0;
    virtual TrkBaseNode* createDkfNode(void) const = 0;

  protected:
    std::unique_ptr<const Surface> m_pSurface;
};

#endif

