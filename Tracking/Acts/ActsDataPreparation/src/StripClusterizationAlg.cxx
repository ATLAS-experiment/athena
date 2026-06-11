/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "StripClusterizationAlg.h"
#include "details/StripRDOCollectionAdapter.h"
#include "details/CellContainer.h"
#include "details/ClusterizationAlg.icc"

namespace ActsTrk {
template class ClusterizationAlg<IStripClusteringTool, false>;
template class ClusterizationAlg<IStripClusteringTool, true>;
}
