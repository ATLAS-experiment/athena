//
// Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/VertexSelectionAlg.h"
#include "TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h"
#include "TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackBiasingAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackExtraVarDecoratorAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackSelectionAlg.h"
#include "TrackingAnalysisAlgorithms/InDetTrackSmearingAlg.h"

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"


// Declare the component(s) of the package:
DECLARE_COMPONENT( CP::VertexSelectionAlg )
DECLARE_COMPONENT( CP::TrackParticleMergerAlg )
DECLARE_COMPONENT( CP::SecVertexTruthMatchAlg )
DECLARE_COMPONENT( CP::InDetTrackBiasingAlg )
DECLARE_COMPONENT( CP::InDetTrackExtraVarDecoratorAlg )
DECLARE_COMPONENT( CP::InDetTrackSelectionAlg )
DECLARE_COMPONENT( CP::InDetTrackSmearingAlg )

