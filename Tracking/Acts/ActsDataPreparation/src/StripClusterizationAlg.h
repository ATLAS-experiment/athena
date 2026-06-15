/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_STRIPCLUSTERIZATIONALG_H
#define ACTSTRK_STRIPCLUSTERIZATIONALG_H
#include <ActsToolInterfaces/IStripClusteringTool.h>
#include "details/StripClusterCacheId.h"
#include "details/ClusterizationAlg.h"

namespace ActsTrk {

class StripClusterizationAlg : public ClusterizationAlg<IStripClusteringTool, false> {
  using ClusterizationAlg<IStripClusteringTool, false>::ClusterizationAlg;
};
  
class StripCacheClusterizationAlg : public ClusterizationAlg<IStripClusteringTool, true> {
  using ClusterizationAlg<IStripClusteringTool, true>::ClusterizationAlg;
};

}
#endif
