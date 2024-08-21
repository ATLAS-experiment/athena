#include "../CreateLumiBlockCollectionFromFile.h"
#include "Root/LumiBlockMetaDataTool.h"

#ifndef XAOD_ANALYSIS
//full-athena-only components
#include "../LumiBlockMuTool.h"
#include "../LumiBlockTester.h"
#include "../LBDurationCondAlg.h"
#include "../LuminosityCondAlg.h"
#include "../BunchCrossingCondAlg.h"
#include "../BunchCrossingIntensityCondAlg.h"
#include "../BunchCrossingAverageCondAlg.h"
#include "../TrigLiveFractionCondAlg.h"
#include "../BunchCrossingCondTest.h"
#include "../BunchCrossingIntensityCondTest.h"
#include "../BunchCrossingAverageCondTest.h"
#include "../LumiBlockMuWriter.h"
#endif

DECLARE_COMPONENT( CreateLumiBlockCollectionFromFile )
#ifndef XAOD_ANALYSIS
DECLARE_COMPONENT( LumiBlockTester )
DECLARE_COMPONENT( LumiBlockMuTool )
DECLARE_COMPONENT( LBDurationCondAlg )
DECLARE_COMPONENT( LuminosityCondAlg )
DECLARE_COMPONENT( BunchCrossingCondAlg )
DECLARE_COMPONENT( BunchCrossingIntensityCondAlg )
DECLARE_COMPONENT( BunchCrossingAverageCondAlg )
DECLARE_COMPONENT( TrigLiveFractionCondAlg )
DECLARE_COMPONENT ( BunchCrossingCondTest )
DECLARE_COMPONENT ( BunchCrossingIntensityCondTest )
DECLARE_COMPONENT ( BunchCrossingAverageCondTest )
DECLARE_COMPONENT(LumiBlockMuWriter)
#endif
DECLARE_COMPONENT( LumiBlockMetaDataTool )


