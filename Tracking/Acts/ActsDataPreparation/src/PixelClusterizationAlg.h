/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <ActsToolInterfaces/IPixelClusteringTool.h>

#include "ClusterizationAlg.h"

namespace ActsTrk {

class PixelClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool> {
    using ClusterizationAlg<IPixelClusteringTool>::ClusterizationAlg;
};

class PixelClusterCacheFillerAlg: public Cache::ViewFillerAlg<xAOD::PixelClusterContainer>{
    using Cache::ViewFillerAlg<xAOD::PixelClusterContainer>::ViewFillerAlg;
};

}