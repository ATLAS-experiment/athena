#include "InDetTrackSystematicsTools/InDetTrackSmearingTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackBiasingTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "InDetTrackSystematicsTools/InclusiveTrackFilterTool.h"
#include "InDetTrackSystematicsTools/TrackFilterTool.h"

#ifndef XAOD_STANDALONE
#include "../InDetTrackSmearingToolTester.h"
#include "../InDetTrackBiasingToolTester.h"
#include "../TrackSystematicsAlg.h"
#include "../TrackSmearingAlg.h"
#include "../JetTrackFilteringAlg.h"
#include "../FakeRateFilterAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( InDet::InDetTrackSmearingTool )
DECLARE_COMPONENT( InDet::InDetTrackTruthOriginTool )
DECLARE_COMPONENT( InDet::InDetTrackTruthFilterTool )
DECLARE_COMPONENT( InDet::InDetTrackBiasingTool )
DECLARE_COMPONENT( InDet::JetTrackFilterTool )
DECLARE_COMPONENT( InDet::InclusiveTrackFilterTool )
DECLARE_COMPONENT( InDet::TrackFilterTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( InDet::InDetTrackSmearingToolTester )
DECLARE_COMPONENT( InDet::InDetTrackBiasingToolTester )
DECLARE_COMPONENT( InDet::TrackSystematicsAlg )
DECLARE_COMPONENT( InDet::TrackSmearingAlg )
DECLARE_COMPONENT( InDet::JetTrackFilteringAlg )
DECLARE_COMPONENT( InDet::FakeRateFilterAlg )

#endif
