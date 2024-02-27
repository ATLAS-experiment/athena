/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <ActsToolInterfaces/IStripClusteringTool.h>

#include "ClusterizationAlg.h"

namespace ActsTrk {

class StripClusterizationAlg : public ClusterizationAlg<IStripClusteringTool> {
    using ClusterizationAlg<IStripClusteringTool>::ClusterizationAlg;
};

class StripClusterCacheFillerAlg: public Cache::ViewFillerAlg<xAOD::StripClusterContainer>{
    using Cache::ViewFillerAlg<xAOD::StripClusterContainer>::ViewFillerAlg;
};
}