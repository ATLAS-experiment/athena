#include "PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h"

#ifndef XAOD_STANDALONE
#include "../testAthenaPhotonAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( ADD_CMODE(CP)::AsgPhotonEfficiencyCorrectionTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( ADD_CMODE(CP)::testAthenaPhotonAlg )
#endif
