#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"

#ifndef XAOD_STANDALONE
#include "../testAthenaEgammaCalibTool.h"
#include "../CalibratedEgammaProvider.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( CP::EgammaCalibrationAndSmearingTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( testAthenaEgammaCalibTool )
DECLARE_COMPONENT( CP::CalibratedEgammaProvider )



#include "../dumpAllSystematics.h"
DECLARE_COMPONENT( DumpAllSystematics )
#endif

