#include "egammaMVACalib/egammaMVASvc.h"
#include "egammaMVACalib/egammaMVACalibTool.h"
#include "egammaMVACalib/egammaTransformerCalibTool.h"

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( egammaMVASvc )
DECLARE_COMPONENT( egammaMVACalibTool )
#ifndef XAOD_ANALYSIS
DECLARE_COMPONENT( egammaTransformerCalibTool )
#endif
