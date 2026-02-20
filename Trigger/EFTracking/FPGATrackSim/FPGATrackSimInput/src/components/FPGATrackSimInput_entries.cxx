#include "../FPGATrackSimRawHitsWrapperAlg.h"
#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"
#include "../FPGATrackSimInputHeaderTool.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "../FPGATrackSimDetectorTool.h"
#include "../FPGATrackSimDumpDetStatusAlgo.h"
#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"
#include "../FPGATrackSimDumpOutputStatAlg.h"


DECLARE_COMPONENT( FPGATrackSimDetectorTool )
DECLARE_COMPONENT( FPGATrackSimReadRawRandomHitsTool )
DECLARE_COMPONENT( FPGATrackSimInputHeaderTool )
DECLARE_COMPONENT( FPGATrackSimRawToLogicalHitsTool )
DECLARE_COMPONENT( FPGATrackSimOutputHeaderTool )

DECLARE_COMPONENT( FPGATrackSimDumpDetStatusAlgo )
DECLARE_COMPONENT( FPGATrackSimRawHitsWrapperAlg )
DECLARE_COMPONENT( FPGATrackSimDumpOutputStatAlg )

