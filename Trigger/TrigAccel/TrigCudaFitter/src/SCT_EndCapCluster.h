// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __SCT_ENDCAP_CLUSTER_H__
#define __SCT_ENDCAP_CLUSTER_H__

#include "SCT_Cluster.h"

class SCT_EndCapCluster : public SCT_Cluster
{
  public:
    SCT_EndCapCluster(const Surface*);
    virtual ~SCT_EndCapCluster(void);

    double m_R;

  public:
    virtual void setParameters(float* par);
    virtual TrkBaseNode* createDkfNode(void) const;
};

#endif
