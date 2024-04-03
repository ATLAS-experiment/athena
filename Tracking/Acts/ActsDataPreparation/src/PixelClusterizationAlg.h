/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <ActsToolInterfaces/IPixelClusteringTool.h>
#include "details/ClusterizationAlg.h"

namespace ActsTrk {

class PixelClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool, false> {
  using ClusterizationAlg<IPixelClusteringTool, false>::ClusterizationAlg;
};

class PixelCacheClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool, true> {
  using ClusterizationAlg<IPixelClusteringTool, true>::ClusterizationAlg;
};

}
