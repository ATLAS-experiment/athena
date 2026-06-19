/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "PhaseIIPixelClusterizationAlg.h"
#include "details/PixelRDOCollectionAdapter.h"
#include "details/CellContainer.h"
#include "details/ClusterizationAlg.icc"

namespace ActsTrk {
template class ClusterizationAlg<IPhaseIIPixelClusteringTool, false>;
template class ClusterizationAlg<IPhaseIIPixelClusteringTool, true>;
}
