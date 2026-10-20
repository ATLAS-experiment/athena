#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"

#ifndef XAOD_STANDALONE
#include "../testAthenaEgammaCalibTool.h"
#include "../CalibratedEgammaProvider.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( ADD_CMODE(CP)::EgammaCalibrationAndSmearingTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( ADD_CMODE(CP)::testAthenaEgammaCalibTool )
DECLARE_COMPONENT( ADD_CMODE(CP)::CalibratedEgammaProvider )



#include "../dumpAllSystematics.h"
DECLARE_COMPONENT( ADD_CMODE(CP)::DumpAllSystematics )
#endif

