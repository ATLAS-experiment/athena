/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <ActsToolInterfaces/IStripClusteringTool.h>
#include "details/ClusterizationAlg.h"

namespace ActsTrk {

class StripClusterizationAlg : public ClusterizationAlg<IStripClusteringTool, false> {
  using ClusterizationAlg<IStripClusteringTool, false>::ClusterizationAlg;
};
  
class StripCacheClusterizationAlg : public ClusterizationAlg<IStripClusteringTool, true> {
  using ClusterizationAlg<IStripClusteringTool, true>::ClusterizationAlg;
};

}
