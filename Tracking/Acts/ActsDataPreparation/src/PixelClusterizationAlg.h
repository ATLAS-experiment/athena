/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <ActsToolInterfaces/IPixelClusteringTool.h>
#include "ClusterizationAlg.h"

namespace ActsTrk {

class PixelClusterizationAlg : public ClusterizationAlg<IPixelClusteringTool> {
    using ClusterizationAlg<IPixelClusteringTool>::ClusterizationAlg;
};

}
