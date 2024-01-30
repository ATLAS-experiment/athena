/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"
#include "src/AmbiguityResolutionAlg.h"
#include "src/ReFitterAlg.h"
#include "src/CompareTrackAlg.h"

// Tools
#include "src/TrackStatePrinter.h"
#include "src/KalmanFitter.h"
#include "src/GaussianSumFitter.h"
#include "src/ProtoTrackCreationAndFitAlg.h"
#include "src/ProtoTrackReportingAlg.h"
#include "src/RandomProtoTrackCreator.h"
#include "src/TruthGuidedProtoTrackCreator.h"
// Algs
DECLARE_COMPONENT( ActsTrk::TrackFindingAlg )
DECLARE_COMPONENT( ActsTrk::ReFitterAlg )
DECLARE_COMPONENT( ActsTrk::CompareTrackAlg )
DECLARE_COMPONENT( ActsTrk::AmbiguityResolutionAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackCreationAndFitAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackReportingAlg )

// Tools
DECLARE_COMPONENT( ActsTrk::TrackStatePrinter )
DECLARE_COMPONENT( ActsTrk::KalmanFitter )
DECLARE_COMPONENT( ActsTrk::GaussianSumFitter )
DECLARE_COMPONENT( ActsTrk::RandomProtoTrackCreator )
DECLARE_COMPONENT( ActsTrk::TruthGuidedProtoTrackCreator )
