#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronEfficiencyCorrection/ElectronChargeEfficiencyCorrectionTool.h"
#include "ElectronEfficiencyCorrection/TElectronTestAlg.h"

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( ADD_CMODE(CP)::AsgElectronEfficiencyCorrectionTool )
DECLARE_COMPONENT( ADD_CMODE(CP)::TElectronTestAlg )
DECLARE_COMPONENT( ADD_CMODE(CP)::ElectronChargeEfficiencyCorrectionTool )
