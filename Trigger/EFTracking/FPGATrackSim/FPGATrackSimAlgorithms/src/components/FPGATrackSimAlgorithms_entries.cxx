#include "../FPGATrackSimLogicalHitsProcessAlg.h"
#include "../FPGATrackSimMapMakerAlg.h"
#include "../FPGATrackSimDataFlowTool.h"
#include "../FPGATrackSimDataPrepAlg.h"
#include "../FPGATrackSimSecondStageAlg.h"
#include "../FPGATrackSimLayerStudyAlg.h"
#include "../FPGATrackSimMergeOutputsAlg.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimRegionMergingAlg.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimTrackFitterTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimNNTrackTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimWindowExtensionTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimNNPathfinderExtensionTool.h"

DECLARE_COMPONENT( FPGATrackSimLogicalHitsProcessAlg )
DECLARE_COMPONENT( FPGATrackSimMapMakerAlg )
DECLARE_COMPONENT( FPGATrackSimNNTrackTool )
DECLARE_COMPONENT( FPGATrackSimOverlapRemovalTool )
DECLARE_COMPONENT( FPGATrackSimTrackFitterTool )
DECLARE_COMPONENT( FPGATrackSimDataFlowTool )
DECLARE_COMPONENT( FPGATrackSimDataPrepAlg )
DECLARE_COMPONENT( FPGATrackSimSecondStageAlg )
DECLARE_COMPONENT( FPGATrackSimWindowExtensionTool )
DECLARE_COMPONENT( FPGATrackSimNNPathfinderExtensionTool )
DECLARE_COMPONENT( FPGATrackSimLayerStudyAlg )
DECLARE_COMPONENT( FPGATrackSim::FPGATrackSimRegionMergingAlg )
DECLARE_COMPONENT ( FPGATrackSimMergeOutputsAlg )
