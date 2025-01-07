#include "../FPGATrackSimLogicalHitsProcessAlg.h"
#include "../FPGATrackSimMapMakerAlg.h"
#include "../FPGATrackSimDataFlowTool.h"
#include "../FPGATrackSimDataPrepAlg.h"
#include "../FPGATrackSimSecondStageAlg.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimTrackFitterTool.h"
#include "../../FPGATrackSimAlgorithms/FPGATrackSimWindowExtensionTool.h"
#include "../../FPGATrackSimAlgorithms//FPGATrackSimNNTrackTool.h"

DECLARE_COMPONENT( FPGATrackSimLogicalHitsProcessAlg )
DECLARE_COMPONENT( FPGATrackSimMapMakerAlg )
DECLARE_COMPONENT( FPGATrackSimNNTrackTool )
DECLARE_COMPONENT( FPGATrackSimOverlapRemovalTool )
DECLARE_COMPONENT( FPGATrackSimTrackFitterTool )
DECLARE_COMPONENT( FPGATrackSimDataFlowTool )
DECLARE_COMPONENT( FPGATrackSimDataPrepAlg )
DECLARE_COMPONENT( FPGATrackSimSecondStageAlg )
DECLARE_COMPONENT( FPGATrackSimWindowExtensionTool )
