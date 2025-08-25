#include "EFTrackingFPGAUtility/FPGADataFormatAlg.h"
#include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
#include "EFTrackingFPGAUtility/TestVectorTool.h"
#include "EFTrackingFPGAUtility/OutputConversionTool.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/xAODSpacePointMaker.h"
#include "EFTrackingFPGAUtility/PassThroughTool.h"
#include "EFTrackingFPGAUtility/EFTrackingDataStreamLoaderAlgorithm.h"
#include "EFTrackingFPGAUtility/EFTrackingDataStreamUnloaderAlgorithm.h"
#include "EFTrackingFPGAUtility/TestVectorGenerator.h"
#include "EFTrackingFPGAUtility/TestVectorChecker.h"

DECLARE_COMPONENT(FPGADataFormatAlg)
DECLARE_COMPONENT(FPGADataFormatTool)
DECLARE_COMPONENT(TestVectorTool)
DECLARE_COMPONENT(OutputConversionTool)
DECLARE_COMPONENT(xAODClusterMaker)
DECLARE_COMPONENT(xAODSpacePointMaker)
DECLARE_COMPONENT(PassThroughTool)
DECLARE_COMPONENT(EFTrackingDataStreamLoaderAlgorithm)
DECLARE_COMPONENT(EFTrackingDataStreamUnloaderAlgorithm)
DECLARE_COMPONENT(EFTrackingFPGAUtility::TestVectorGenerator)
DECLARE_COMPONENT(EFTrackingFPGAUtility::TestVectorChecker)

