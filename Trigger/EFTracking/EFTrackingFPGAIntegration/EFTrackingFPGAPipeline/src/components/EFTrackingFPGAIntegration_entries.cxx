#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAPipeline/PixelClustering.h"
#include "EFTrackingFPGAPipeline/FPGAStripClustering.h"
#include "EFTrackingFPGAPipeline/Spacepoints.h"
#include "EFTrackingFPGAPipeline/EFTrackingXrtAlgorithm.h"
#include "EFTrackingFPGAPipeline/BenchmarkAlg.h"
#include "EFTrackingFPGAPipeline/F600IntegrationAlg.h"

DECLARE_COMPONENT(IntegrationBase)
DECLARE_COMPONENT(PixelClustering)
DECLARE_COMPONENT(FPGAStripClustering)
DECLARE_COMPONENT(Spacepoints)
DECLARE_COMPONENT(DataPreparationPipeline)
DECLARE_COMPONENT(EFTrackingXrtAlgorithm)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::BenchmarkAlg)
DECLARE_COMPONENT(EFTrackingFPGAIntegration::F600IntegrationAlg)

