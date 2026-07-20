/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/HgtdClusterizationAlg.h"
#include "src/HgtdClusteringTool.h"
#include "src/HgtdTimedClusteringTool.h"
#include "src/PixelClusterizationAlg.h"
#include "src/PhaseIIPixelClusterizationAlg.h"
#include "src/StripClusterizationAlg.h"
#include "src/PixelSpacePointFormationAlg.h"
#include "src/StripSpacePointFormationAlg.h"
#include "src/PixelClusteringTool.h"
#include "src/StripClusteringTool.h"
#include "src/PixelSpacePointFormationTool.h"
#include "src/StripSpacePointFormationTool.h"

#include "src/CacheCreator.h"
#include "src/CollectionDataPreparationAlg.h"


// Algs
DECLARE_COMPONENT(ActsTrk::HgtdClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::PixelClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::PixelCacheClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::StripCacheClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::PhaseIIPixelClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::PhaseIIPixelCacheClusterizationAlg)

DECLARE_COMPONENT(ActsTrk::PixelSpacePointFormationAlg)
DECLARE_COMPONENT(ActsTrk::PixelCacheSpacePointFormationAlg)

DECLARE_COMPONENT(ActsTrk::StripSpacePointFormationAlg)
DECLARE_COMPONENT(ActsTrk::StripCacheSpacePointFormationAlg)

// Tools
DECLARE_COMPONENT(ActsTrk::HgtdClusteringTool)
DECLARE_COMPONENT(ActsTrk::HgtdTimedClusteringTool)
DECLARE_COMPONENT(ActsTrk::PixelClusteringTool)
DECLARE_COMPONENT(ActsTrk::PhaseIIPixelClusteringTool)
DECLARE_COMPONENT(ActsTrk::StripClusteringTool)
DECLARE_COMPONENT(ActsTrk::PixelSpacePointFormationTool)
DECLARE_COMPONENT(ActsTrk::StripSpacePointFormationTool)

//Cache related algs
DECLARE_COMPONENT(ActsTrk::Cache::CreatorAlg)

//
DECLARE_COMPONENT(ActsTrk::PixelClusterDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::HgtdClusterDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::SpacePointDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::PixelClusterCacheDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterCacheDataPreparationAlg)
DECLARE_COMPONENT(ActsTrk::SpacePointCacheDataPreparationAlg)
