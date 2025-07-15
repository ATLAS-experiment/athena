/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Algs
#include "src/GridTripletSeedingAlg.h"
#include "src/SeedingAlg.h"

// Tools
#include "src/GbtsSeedingTool.h"
#include "src/GridTripletSeedingTool.h"
#include "src/OrthogonalSeedingTool.h"
#include "src/SeedingTool.h"
#include "src/SiSpacePointsSeedMaker.h"
#include "src/TrackParamsEstimationTool.h"

// Algs
DECLARE_COMPONENT(ActsTrk::SeedingAlg)
DECLARE_COMPONENT(ActsTrk::GridTripletSeedingAlg)

// Tools
DECLARE_COMPONENT(ActsTrk::SeedingTool)
DECLARE_COMPONENT(ActsTrk::OrthogonalSeedingTool)
DECLARE_COMPONENT(ActsTrk::GbtsSeedingTool)
DECLARE_COMPONENT(ActsTrk::SiSpacePointsSeedMaker)
DECLARE_COMPONENT(ActsTrk::TrackParamsEstimationTool)
DECLARE_COMPONENT(ActsTrk::GridTripletSeedingTool)
