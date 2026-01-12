// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __PIXEL_CLUSTER_H__
#define __PIXEL_CLUSTER_H__

#include "SiCluster.h"

#include <memory>

class PixelCluster : public SiCluster
{
  public:
    PixelCluster(std::unique_ptr<const Surface>);

    double m_m[2]{};
    double m_cov[2][2]{};

  public:
    virtual void setParameters(float* par);
    virtual TrkBaseNode* createDkfNode(void) const;
};
#endif
