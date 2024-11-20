// Note: there is another .cxx files with components that don't require
// Open CL For compilation
#include "../DataPreparationPipeline.h"
#include "../IntegrationBase.h"
#include "../PixelClustering.h"
#include "../Spacepoints.h"
#include "../xAODContainerMaker.h"
#include "../TestVectorTool.h"
#include "../PassThroughTool.h"

DECLARE_COMPONENT(IntegrationBase)
DECLARE_COMPONENT(PixelClustering)
DECLARE_COMPONENT(Spacepoints)
DECLARE_COMPONENT(DataPreparationPipeline)
DECLARE_COMPONENT(xAODContainerMaker)
DECLARE_COMPONENT(TestVectorTool)
DECLARE_COMPONENT(PassThroughTool)
