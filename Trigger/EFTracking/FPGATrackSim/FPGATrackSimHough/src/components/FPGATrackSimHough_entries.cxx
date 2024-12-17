#include "../FPGATrackSimEtaPatternFilterTool.h"
#include "../FPGATrackSimHough1DShiftTool.h"
#include "FPGATrackSimHough/FPGATrackSimHoughRootOutputTool.h"
#include "../FPGATrackSimHoughTransformTool.h"
#include "../FPGATrackSimPhiRoadFilterTool.h"
#include "../FPGATrackSimRoadUnionTool.h"
#include "../FPGATrackSimSpacepointRoadFilterTool.h"
#include "../FPGATrackSimGenScanTool.h"
#include "../FPGATrackSimGenScanMonitoring.h"
#include "../FPGATrackSimGenScanBinning.h"


DECLARE_COMPONENT( FPGATrackSimEtaPatternFilterTool )
DECLARE_COMPONENT( FPGATrackSimHough1DShiftTool )
DECLARE_COMPONENT( FPGATrackSimHoughRootOutputTool )
DECLARE_COMPONENT( FPGATrackSimHoughTransformTool )
DECLARE_COMPONENT( FPGATrackSimPhiRoadFilterTool )
DECLARE_COMPONENT( FPGATrackSimRoadUnionTool )
DECLARE_COMPONENT( FPGATrackSimSpacepointRoadFilterTool )
DECLARE_COMPONENT( FPGATrackSimGenScanTool )
DECLARE_COMPONENT( FPGATrackSimGenScanMonitoring)
DECLARE_COMPONENT( FPGATrackSimGenScanPhiSlicedKeyLyrBinning )
DECLARE_COMPONENT( FPGATrackSimGenScanKeyLyrBinning )
DECLARE_COMPONENT( FPGATrackSimGenScanStdTrkBinning )
