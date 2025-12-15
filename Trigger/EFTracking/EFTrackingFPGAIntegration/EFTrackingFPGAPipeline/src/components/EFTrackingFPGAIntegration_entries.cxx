#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAPipeline/PixelClustering.h"
#include "EFTrackingFPGAPipeline/FPGAStripClustering.h"
#include "EFTrackingFPGAPipeline/Spacepoints.h"
#include "EFTrackingFPGAPipeline/EFTrackingXrtAlgorithm.h"
#include "EFTrackingFPGAPipeline/F1X0IntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F1X0XRTIntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F100StreamIntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F150KernelTesterAlg.h"
#include "EFTrackingFPGAPipeline/F150IntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F110IntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F110StreamIntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F100DataEncodingAlg.h"
#include "EFTrackingFPGAPipeline/F100EDMConversionAlg.h"
#include "EFTrackingFPGAPipeline/F600IntegrationAlg.h"
#include "EFTrackingFPGAPipeline/F150EDMConversionAlg.h"

DECLARE_COMPONENT(IntegrationBase)
DECLARE_COMPONENT(PixelClustering)
DECLARE_COMPONENT(FPGAStripClustering)
DECLARE_COMPONENT(Spacepoints)
DECLARE_COMPONENT(DataPreparationPipeline)
DECLARE_COMPONENT(EFTrackingXrtAlgorithm)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F1X0IntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F1X0XRTIntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F100StreamIntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F110IntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F150IntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F110StreamIntegrationAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F150KernelTesterAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F100EDMConversionAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F150EDMConversionAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F100DataEncodingAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F600IntegrationAlg)

