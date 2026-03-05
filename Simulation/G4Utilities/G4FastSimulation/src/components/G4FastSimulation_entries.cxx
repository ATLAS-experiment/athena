#include "../DeadMaterialShowerTool.h"
#include "../SimpleFastKillerTool.h"
#ifndef SIMULATIONBASE
#include "../FastCaloSimTool.h"
#include "../FatrasG4Tool.h"
#endif

DECLARE_COMPONENT( DeadMaterialShowerTool )
DECLARE_COMPONENT( SimpleFastKillerTool )
#ifndef SIMULATIONBASE
DECLARE_COMPONENT( FastCaloSimTool )
DECLARE_COMPONENT( FatrasG4Tool )
#endif
