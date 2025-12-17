// Top level tool
#include "METUtilities/METMaker.h"
#include "METUtilities/METSystematicsTool.h"
#include "METUtilities/METSignificance.h"
#include "METUtilities/METNet.h"
#include "METUtilities/ColumnarMETMaker.h"
#include "METUtilities/METNetSig.h"
// Algs
#ifndef XAOD_STANDALONE
#include "../METMakerAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

using namespace met;

DECLARE_COMPONENT( METMaker )
DECLARE_COMPONENT( METSystematicsTool )
DECLARE_COMPONENT( METSignificance )
DECLARE_COMPONENT( METNet )
DECLARE_COMPONENT( METNetSig )
DECLARE_COMPONENT( ColumnarMETMaker )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( METMakerAlg )
#endif
