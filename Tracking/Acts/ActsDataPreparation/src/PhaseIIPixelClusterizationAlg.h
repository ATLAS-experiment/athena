/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_PHASEIIPIXELCLUSTERIZATIONALG_H
#define ACTSTRK_PHASEIIPIXELCLUSTERIZATIONALG_H

#include <ActsToolInterfaces/IPhaseIIPixelClusteringTool.h>
#include "details/PixelClusterCacheId.h"
#include "details/CellContainer.h"
#include "details/ClusterizationAlg.h"

namespace ActsTrk {

class PhaseIIPixelClusterizationAlg : public ClusterizationAlg<IPhaseIIPixelClusteringTool, false> {
  using ClusterizationAlg<IPhaseIIPixelClusteringTool, false>::ClusterizationAlg;
};

class PhaseIIPixelCacheClusterizationAlg : public ClusterizationAlg<IPhaseIIPixelClusteringTool, true> {
  using ClusterizationAlg<IPhaseIIPixelClusteringTool, true>::ClusterizationAlg;
};

}
#endif
