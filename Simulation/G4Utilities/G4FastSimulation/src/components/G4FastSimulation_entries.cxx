#include "../DeadMaterialShowerTool.h"
#include "../SimpleFastKillerTool.h"
#ifndef SIMULATIONBASE
#include "../FastCaloSimTool.h"
#include "../FatrasG4Tool.h"
#include "../AFatrasG4Tool.h"
#include "../FatrasG4PhysicsTool.h"
#endif

DECLARE_COMPONENT( DeadMaterialShowerTool )
DECLARE_COMPONENT( SimpleFastKillerTool )
#ifndef SIMULATIONBASE
DECLARE_COMPONENT( FastCaloSimTool )
DECLARE_COMPONENT( FatrasG4Tool )
DECLARE_COMPONENT( AFatrasG4Tool )
DECLARE_COMPONENT( FatrasG4PhysicsTool )
DECLARE_COMPONENT( GammaConversionOnlyPhysicsTool )
#endif
