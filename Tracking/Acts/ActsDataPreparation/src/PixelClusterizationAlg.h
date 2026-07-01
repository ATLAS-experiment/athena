/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_PIXELCLUSTERIZATIONALG_H
#define ACTSTRK_PIXELCLUSTERIZATIONALG_H

#include <ActsToolInterfaces/IPixelClusteringTool.h>
#include "details/PixelClusterCacheId.h"
#include "details/CellContainer.h"
#include "details/ClusterizationAlg.h"

namespace ActsTrk {

class PixelClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool, false> {
  using ClusterizationAlg<IPixelClusteringTool, false>::ClusterizationAlg;
};

class PixelCacheClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool, true> {
  using ClusterizationAlg<IPixelClusteringTool, true>::ClusterizationAlg;
};

}
#endif
