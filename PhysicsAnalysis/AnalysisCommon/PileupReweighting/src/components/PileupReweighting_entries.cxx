#include "PileupReweighting/PileupReweightingTool.h"

#ifndef XAOD_STANDALONE
#include "../PileupReweightingProvider.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( CP::PileupReweightingTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( CP::PileupReweightingProvider )
#endif
