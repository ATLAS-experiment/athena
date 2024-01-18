/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SeedingAlg.h"
#include "src/TrackFindingAlg.h"
#include "src/AmbiguityResolutionAlg.h"
#include "src/ActsReFitterAlg.h"
#include "src/ActsCompareTrackAlg.h"

// Tools
#include "src/SeedingTool.h"
#include "src/OrthogonalSeedingTool.h"
#include "src/SiSpacePointsSeedMaker.h"
#include "src/TrackParamsEstimationTool.h"
#include "src/TrackStatePrinter.h"
#include "src/ActsKalmanFitter.h"
#include "src/ActsGaussianSumFitter.h"

// Algs
DECLARE_COMPONENT( ActsTrk::SeedingAlg )
DECLARE_COMPONENT( ActsTrk::TrackFindingAlg )
DECLARE_COMPONENT( ActsTrk::ActsReFitterAlg )
DECLARE_COMPONENT( ActsTrk::ActsCompareTrackAlg )
DECLARE_COMPONENT( ActsTrk::AmbiguityResolutionAlg )

// Tools
DECLARE_COMPONENT( ActsTrk::SeedingTool )
DECLARE_COMPONENT( ActsTrk::OrthogonalSeedingTool )
DECLARE_COMPONENT( ActsTrk::SiSpacePointsSeedMaker )
DECLARE_COMPONENT( ActsTrk::TrackParamsEstimationTool )
DECLARE_COMPONENT( ActsTrk::TrackStatePrinter )
DECLARE_COMPONENT( ActsTrk::ActsKalmanFitter )
DECLARE_COMPONENT( ActsTrk::ActsGaussianSumFitter )
