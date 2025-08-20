#include "InDetTrackSystematicsTools/InDetTrackSmearingTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackBiasingTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "InDetTrackSystematicsTools/InclusiveTrackFilterTool.h"

#ifndef XAOD_STANDALONE
#include "../InDetTrackSmearingToolTester.h"
#include "../TrackSystematicsAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( InDet::InDetTrackSmearingTool )
DECLARE_COMPONENT( InDet::InDetTrackTruthOriginTool )
DECLARE_COMPONENT( InDet::InDetTrackTruthFilterTool )
DECLARE_COMPONENT( InDet::InDetTrackBiasingTool )
DECLARE_COMPONENT( InDet::JetTrackFilterTool )
DECLARE_COMPONENT( InDet::InclusiveTrackFilterTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( InDet::InDetTrackSmearingToolTester )
DECLARE_COMPONENT( InDet::TrackSystematicsAlg )
#endif
