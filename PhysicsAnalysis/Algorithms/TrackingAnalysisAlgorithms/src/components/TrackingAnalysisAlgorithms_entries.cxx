//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/VertexSelectionAlg.h"
#include "TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h"
#include "TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h"
#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationAlg.h"
#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationTool.h"
#include "TrackingAnalysisAlgorithms/InDetTrackBiasingAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackMomentumDecoratorAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackSelectionAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackSmearingAlg.h"

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

// Declare the component(s) of the package:
DECLARE_COMPONENT( CP::VertexSelectionAlg )
DECLARE_COMPONENT( CP::TrackParticleMergerAlg )
DECLARE_COMPONENT( CP::SecVertexTruthMatchAlg )
DECLARE_COMPONENT( CP::PixelDEdxEqualizationAlg )
DECLARE_COMPONENT( CP::PixelDEdxEqualizationTool )
DECLARE_COMPONENT( CP::InDetTrackBiasingAlg )
DECLARE_COMPONENT( CP::InDetTrackMomentumDecoratorAlg )
DECLARE_COMPONENT( CP::InDetTrackSelectionAlg )
DECLARE_COMPONENT( CP::InDetTrackSmearingAlg )
