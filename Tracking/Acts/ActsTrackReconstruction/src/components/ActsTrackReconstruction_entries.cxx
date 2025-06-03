/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"
#include "src/AmbiguityResolutionAlg.h"
#include "src/ScoreBasedAmbiguityResolutionAlg.h"
#include "src/ReFitterAlg.h"
#include "src/TrackToTrackParticleCnvAlg.h"
#include "src/ProtoTrackCreationAndFitAlg.h"
#include "src/TrackExtensionAlg.h"
#include "src/ProtoTrackReportingAlg.h"

// Tools
#include "src/ITkAnalogueClusteringTool.h"
#include "src/TrackStatePrinterTool.h"
#include "src/KalmanFitterTool.h"
#include "src/GaussianSumFitterTool.h"
#include "src/GlobalChiSquareFitterTool.h"
#include "src/RandomProtoTrackCreatorTool.h"
#include "src/TruthGuidedProtoTrackCreatorTool.h"

// Algs
DECLARE_COMPONENT( ActsTrk::TrackFindingAlg )
DECLARE_COMPONENT( ActsTrk::TrackFindingBaseAlg )
DECLARE_COMPONENT( ActsTrk::ReFitterAlg )
DECLARE_COMPONENT( ActsTrk::AmbiguityResolutionAlg )
DECLARE_COMPONENT( ActsTrk::ScoreBasedAmbiguityResolutionAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackCreationAndFitAlg )
DECLARE_COMPONENT( ActsTrk::TrackExtensionAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackReportingAlg )
DECLARE_COMPONENT( ActsTrk::TrackToTrackParticleCnvAlg )

// Tools
DECLARE_COMPONENT( ActsTrk::ITkAnalogueClusteringTool )
DECLARE_COMPONENT( ActsTrk::TrackStatePrinterTool )
DECLARE_COMPONENT( ActsTrk::KalmanFitterTool )
DECLARE_COMPONENT( ActsTrk::GaussianSumFitterTool )
DECLARE_COMPONENT( ActsTrk::GlobalChiSquareFitterTool )
DECLARE_COMPONENT( ActsTrk::RandomProtoTrackCreatorTool )
DECLARE_COMPONENT( ActsTrk::TruthGuidedProtoTrackCreatorTool )
