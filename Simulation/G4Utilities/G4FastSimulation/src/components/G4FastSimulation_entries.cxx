#include "../DeadMaterialShowerTool.h"
#include "../SimpleFastKillerTool.h"
#include "../FastCaloSimTool.h"
#include "../CaloCellContainerSDTool.h"
#ifndef SIMULATIONBASE
#include "../FastCaloSimParamHitAnalysis.h"
#include "../FatrasG4Tool.h"
#include "../AFatrasG4Tool.h"
#endif

DECLARE_COMPONENT( DeadMaterialShowerTool )
DECLARE_COMPONENT( SimpleFastKillerTool )
DECLARE_COMPONENT( FastCaloSimTool )
DECLARE_COMPONENT( CaloCellContainerSDTool )
#ifndef SIMULATIONBASE
DECLARE_COMPONENT( FastCaloSimParamHitAnalysis )
DECLARE_COMPONENT( FatrasG4Tool )
DECLARE_COMPONENT( AFatrasG4Tool )
#endif
