/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelClusterizationAlg.h"
#include "src/StripClusterizationAlg.h"
#include "src/PixelSpacePointFormationAlg.h"
#include "src/StripSpacePointFormationAlg.h"
#include "src/PixelClusteringTool.h"
#include "src/StripClusteringTool.h"
#include "src/PixelSpacePointFormationTool.h"
#include "src/CoreStripSpacePointFormationTool.h"
#include "src/StripSpacePointFormationTool.h"

#include "src/CacheCreator.h"

// Algs
DECLARE_COMPONENT(ActsTrk::PixelClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::PixelSpacePointFormationAlg)
DECLARE_COMPONENT(ActsTrk::StripSpacePointFormationAlg)
// Tools
DECLARE_COMPONENT(ActsTrk::PixelClusteringTool)
DECLARE_COMPONENT(ActsTrk::StripClusteringTool)
DECLARE_COMPONENT(ActsTrk::PixelSpacePointFormationTool)
DECLARE_COMPONENT(ActsTrk::CoreStripSpacePointFormationTool)
DECLARE_COMPONENT(ActsTrk::StripSpacePointFormationTool)

//Cache related algs
DECLARE_COMPONENT(ActsTrk::Cache::CreatorAlg)
DECLARE_COMPONENT(ActsTrk::PixelClusterCacheFillerAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterCacheFillerAlg)
