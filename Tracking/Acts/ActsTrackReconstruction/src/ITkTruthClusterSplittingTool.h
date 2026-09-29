/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ITKTRUTHCLUSTERSPLITTINGTOOL_H
#define ACTSTRACKRECONSTRUCTION_ITKTRUTHCLUSTERSPLITTINGTOOL_H

#include "src/detail/TruthClusterSplittingToolImpl.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {

  class ITkTruthClusterSplittingTool :
    public detail::TruthClusterSplittingToolImpl<detail::RecoTrackStateContainer> {
  public:
    using traj_t = detail::RecoTrackStateContainer;

    using detail::TruthClusterSplittingToolImpl<traj_t>::TruthClusterSplittingToolImpl;
  };

} // namespace ActsTrk


#endif
