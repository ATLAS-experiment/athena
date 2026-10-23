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

DECLARE_COMPONENT( ADD_CMODE(met)::METMaker )
DECLARE_COMPONENT( ADD_CMODE(met)::METSystematicsTool )
DECLARE_COMPONENT( ADD_CMODE(met)::METSignificance )
DECLARE_COMPONENT( ADD_CMODE(met)::METNet )
DECLARE_COMPONENT( ADD_CMODE(met)::METNetSig )
DECLARE_COMPONENT( ADD_CMODE(met)::ColumnarMETMaker )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( ADD_CMODE(met)::METMakerAlg )
#endif
