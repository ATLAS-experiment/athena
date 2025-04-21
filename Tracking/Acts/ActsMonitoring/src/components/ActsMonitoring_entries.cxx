/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Algs
#include "src/PixelClusterAnalysisAlg.h"
#include "src/StripClusterAnalysisAlg.h"
#include "src/HgtdClusterAnalysisAlg.h"
#include "src/SpacePointAnalysisAlg.h"
#include "src/SeedAnalysisAlg.h"
#include "src/EstimatedTrackParamsAnalysisAlg.h"
#include "src/SeedingAlgorithmAnalysisAlg.h"
#include "src/TrackAnalysisAlg.h"
#include "src/TrackParticleAnalysisAlg.h"
#include "src/SeedsToTrackParamsAlg.h"
#include "src/ITkAlignMonResidualsAlg.h"
#include "src/ActsInspectTruthContentAlg.h"
// Tools
#include "src/PhysValTool.h"

// Algs
DECLARE_COMPONENT( ActsTrk::PixelClusterAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::StripClusterAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::HgtdClusterAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::SpacePointAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::SeedAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::SeedingAlgorithmAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::EstimatedTrackParamsAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::TrackAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::TrackParticleAnalysisAlg )
DECLARE_COMPONENT( ActsTrk::SeedsToTrackParamsAlg )
DECLARE_COMPONENT( ActsTrk::ITkAlignMonResidualsAlg )
DECLARE_COMPONENT( ActsTrk::ActsInspectTruthContentAlg )
// Tools
DECLARE_COMPONENT( ActsTrk::PhysValTool )
