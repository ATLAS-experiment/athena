// Note: there is another .cxx files with components that don't require
// Open CL For compilation
#include "../DataPreparationPipeline.h"
#include "../IntegrationBase.h"
#include "../PixelClustering.h"
#include "../Spacepoints.h"
#include "../PassThroughTool.h"
#include "../EFTrackingXrtAlgorithm.h"
#include "../EFTrackingDataStreamLoaderAlgorithm.h"
#include "../EFTrackingDataStreamUnloaderAlgorithm.h"

DECLARE_COMPONENT(IntegrationBase)
DECLARE_COMPONENT(PixelClustering)
DECLARE_COMPONENT(Spacepoints)
DECLARE_COMPONENT(DataPreparationPipeline)
DECLARE_COMPONENT(PassThroughTool)
DECLARE_COMPONENT(EFTrackingXrtAlgorithm)
DECLARE_COMPONENT(EFTrackingDataStreamLoaderAlgorithm)
DECLARE_COMPONENT(EFTrackingDataStreamUnloaderAlgorithm)
